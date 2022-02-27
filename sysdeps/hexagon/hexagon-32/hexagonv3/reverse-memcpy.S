/* Optimized reverse-memcpy implementation for Hexagon V3 core.
 * Copyright (c) 2009, 2010, 2011 QUALCOMM INCORPORATED.
 * All Rights Reserved.
 */

#include <sysdep.h>
#include <bp-sym.h>

/* --------------------------------------------------------------------
  Function

     Reverse mem copy, used as part of the memmove function.

     void * reverse_memcpy(char *, char *, int)

  Description

     Library function for memcpy where length bytes are copied from
     ptr_in to ptr_out. ptr_out is returned unchanged.
     Allows any combination of alignment on input and output pointers
     and length from 0 to 2^32-1.

  Restrictions

     The arrays can overlap, the program requires that the source
     address is lower in the address space than the destination
     address.

     For blocks less than 16 bytes a byte by byte copy is performed for
     8 byte alignments, and length multiples a dword copy is performed
     up to 96 bytes.

     The program will underfetch below the  start of the array by up to
     15 bytes, so the function must be called twice len-15 and 15. If
     the block is known to be within a page this will not be a page
     fault.

     For large blocks the code has no dczero capability as the input
     and output may overlap and so this would potentially destroy data.

  Natural c model
  ===============
  void * reverse_memcpy(char * ptr_out, char * ptr_in, int len)
  {
   int i;
   for(i=len-1; i >= 0; i--) { ptr_out[i] = ptr_in[i]; }
   return(ptr_out);
  }
  Optimized C reverse memcpy function
  ====================================
  void * reverse_memcpy(char * ptr_out, char * ptr_in, int len) {
   int i;
   int epilog, epilog8, kernel, prolog, prolog8;
   s32 mask;
   int offset;
   s64 data0, dataF8, data70;

   s64 * ptr8_in, * ptr8_out;
   s32 * ptr4;
   s16 * ptr2;

   ptr8_in = (s64 *) ((((int) ptr_in) + len) & 0xFFFFFFF8);
   offset = (((int) ptr_in) + ln) & 7;

   epilog = 32 - ((int) ptr_out) ;
   mask  = 0x7fffffff >> HEXAGON_R_cl0_R(len);
   epilog = epilog & mask ;
   kernel = len - epilog;
   prolog = kernel & 0x1F;
   kernel = kernel>>5;

   dataF8 = *ptr8_in--;
   data70 = *ptr8_in--;
   data0 = HEXAGON_P_valignb_PPp(dataF8, data70, offset);

   ptr_out += len;

   if (prolog & 1) {
     ptr_out[-1] = (u8) (data0>>56); data0 <<= 8; ptr_out-=1;
   }
   if(prolog & 2) {
     ptr2 = (s16 *) &ptr_out[-2]; ptr_out -= 2;
     ptr2[0] = (u16) (data0 >> 48); data0 <<= 16;;
   }
   if(prolog & 4) {
     ptr4 = (s32 *) &ptr_out[-4]; ptr_out -= 4;
     ptr4[0] = (u32) (data0 >> 32); data0 <<= 32;
   }
   offset = offset - (prolog & 7);
   if (offset < 0) {
     dataF8 = data70;
     data70 = *ptr8_in--;
   }
   offset = offset & 0x7;

   prolog8 = prolog >> 3;
   if (prolog8) { ptr_out -= 8; for (i=0; i < prolog8; i++) {
       data0 = HEXAGON_P_valignb_PPp(dataF8,data70, offset);
       ptr8_out = (s64 *) &ptr_out[0]; *ptr8_out = data0; ptr_out -= 8;

       dataF8 = data70 ;
       data70 = *ptr8_in--;
   }   ptr_out += 8; }
   if(kernel) { ptr_out -= 8; for(i=0; i < kernel; i++) {
       data0 = HEXAGON_P_valignb_PPp(dataF8, data70, offset);
       ptr8_out = (s64 *) &ptr_out[0]; *ptr8_out = data0; ptr_out -= 8;
       dataF8 = *ptr8_in--;

       data0 = HEXAGON_P_valignb_PPp(data70, dataF8, offset);
       ptr8_out = (s64 *) &ptr_out[0]; *ptr8_out = data0; ptr_out -= 8;
       data70 = *ptr8_in--;

       data0 = HEXAGON_P_valignb_PPp(dataF8, data70, offset);
       ptr8_out = (s64 *) &ptr_out[0]; *ptr8_out = data0; ptr_out -= 8;
       dataF8 = *ptr8_in--;

       data0 = HEXAGON_P_valignb_PPp(data70, dataF8, offset);
       ptr8_out = (s64 *) &ptr_out[0]; *ptr8_out = data0; ptr_out -= 8;
       data70 = *ptr8_in--;
   }   ptr_out += 8; }
   epilog8 = epilog >> 3;
   if (epilog8) { ptr_out -= 8; for (i=0; i < epilog8; i++) {
       data0 = HEXAGON_P_valignb_PPp(dataF8, data70, offset);
       ptr8_out = (s64 *) &ptr_out[0]; *ptr8_out = data0; ptr_out -= 8;
       dataF8 = data70;
       data70 = *ptr8_in--;
   }   ptr_out += 8; }
   data0 = HEXAGON_P_valignb_PPp(dataF8, data70, offset);

   if(epilog & 4) {
     ptr4 = (s32 *) &ptr_out[-4]; ptr_out -= 4;
     ptr4[0] = (u32) (data0 >> 32); data0 <<= 32;
   }
   if(epilog & 2) {
     ptr2 = (s16 *) &ptr_out[-2]; ptr_out -= 2;
     ptr2[0] = (u16) (data0 >> 48); data0 <<= 16;
   }
   if (epilog & 1) { ptr_out[-1] = (u8)(data0 >> 56); ptr_out--;}
   ptr8_in++;
   return(ptr_out);
  }

 ------------------------------------------------------------------ */

#define ptr_out            R0
#define ptr_in             R1
#define len                R2
#define data70             R13:12
#define dataF8             R11:10
#define ldata0             R7:6
#define ldata1             R19:18
#define data1              R7
#define data0              R6
#define d0                 R18
#define ifbyte             p0
#define ifhword            p0
#define ifword             p0
#define noprolog           p0
#define noepilog           p0
#define align              p2
#define kernel0            p1
#define dalign             R19
#define rest               R8
#define star3              R16
#define back               R7
#define epilog             R3
#define inc                R15:14
#define ptr_in_p_128       R5
#define kernel             R4
#define ptr_in_p_128kernel R5:4
#define mask               R8
#define shift              R8
#define shift2             R5
#define prolog             R15
#define epilogdws          R15
#define shiftb             R14
#define offset             R9
#define align888           R14
#define len8               R9

/* --------------------------------------------------------------- */

ENTRY(BP_SYM(_hexagon_reverse_memcpy))
CALL_MCOUNT
{
	p2 = cmp.eq(len, #0)              // =0
	align888 = or(ptr_in, ptr_out)    // %8 < 97
	p0 = cmp.gt(len, #23)             // %1, <24
	p1 = cmp.eq(ptr_in, ptr_out)      // attempt to overwrite self
}
{
	p1 = or(p2, p1)
	p3 = cmp.gt(len, #95)             // %8 < 97
	align888 = or(align888, len)      // %8 < 97
	len8 = lsr(len, #3)               // %8 < 97
}
{
	if(!p1) ptr_in = add(ptr_in, len)
	epilog = sub(#32, ptr_out)        // all bytes before line multiples of data
	p2 = bitsclr(align888, #7)        // %8 < 97
	if(p1) jumpr r31                  // =0
}
{
	ptr_in_p_128 = add(ptr_in, #-32)
	ptr_out = add(ptr_out, len)
	p2 = and(p2,!p3)                  // %8 < 97
	if(p2.new) jump:NT .Ldwordaligned // %8 < 97
}
{
	dcfetch(ptr_in_p_128)
	if(!p0) jump  .Lshort_bytecpy      // %1, <24
	mask.l = #LO(0x7fffffff);
}
// //////////////////////////////////////////////////
.cfi_def_cfa 30, 8
.cfi_offset 31, -4
.cfi_offset 30, -8
#if defined(PIC) || defined(__PIC__)
   .cfi_offset 16, -32
   .cfi_offset 17, -28
   .cfi_offset 18, -24
   .cfi_offset 19, -20
   .cfi_offset 24, -16
#else
   .cfi_offset 16, -24
   .cfi_offset 17, -20
   .cfi_offset 18, -16
   .cfi_offset 19, -12
#endif // __PIC__
{
#if defined(PIC) || defined(__PIC__)
	allocframe(#24)                   // save r31 on stack
#else
	allocframe(#16)                   // save r31 on stack
#endif
	mask.h = #HI(0x7fffffff);
	ptr_in_p_128 = add(ptr_in_p_128, #-32)
	back = cl0(len)
}
{
	memd(sp+#0) = R17:16              // save r16,r17 on stack6
#if defined(PIC) || defined(__PIC__)
	r31.l = #LO(.Lmemcpy_return@GOTREL) // set up final return pointer
#else
	r31.l = #LO(.Lmemcpy_return)      // set up final return pointer
#endif
	epilog &= lsr(mask, back)
	offset = and(ptr_in, #7)
}
{
	kernel = sub(len, epilog)
	memd(sp+#8) = R19:18              // save r18,r19 on stack
	dalign = sub(ptr_out, ptr_in)
#if defined(PIC) || defined(__PIC__)
	r31.h = #HI(.Lmemcpy_return@GOTREL) // set up final return pointer
#else
	r31.h = #HI(.Lmemcpy_return)      // set up final return pointer
#endif
}
{
	prolog = and(kernel, #31);
	noprolog = bitsclr(kernel, #7)
	dcfetch(ptr_in_p_128)
	ptr_in_p_128 = add(ptr_in_p_128, #-32)
}
{
	ptr_in = and(ptr_in, #-8)             //  & 0xFFFFFFF8
	shift = asl(prolog, #3)
	star3 = and(prolog, #7)
#if defined(PIC) || defined(__PIC__)
	memw(sp+#16) = R24
#endif
}
.Lpc:
{
#if defined(PIC) || defined(__PIC__)
	r10 = pc
#endif
	prolog = lsr(prolog, #3)     // number of dword in prolog
	shiftb = and(shift, #8)
	back = add(len, offset)
}
{
	p3 = cmp.gt(back, #8)
	kernel = lsr(kernel, #5)
	dcfetch(ptr_in_p_128)
	ptr_in_p_128 = add(ptr_in_p_128, #-32)
}
{
	p1 = cmp.eq(prolog, #0)
	if(!p1.new) prolog = add(prolog, #1)
	dcfetch(ptr_in_p_128)     // reserve the line 64bytes on
	ptr_in_p_128 = add(ptr_in_p_128, #-32)
}
{
	dcfetch(ptr_in_p_128)     // reserve the line 64bytes on
	ptr_in_p_128 = add(ptr_in_p_128, #-32)
#if defined(PIC) || defined(__PIC__)
	r24.h = #HI(.Lpc@GOTREL)     // Set up GOT pointer
#endif
}
{
	dcfetch(ptr_in_p_128)     // reserve the line 64bytes on
	ptr_in_p_128 = add(ptr_in_p_128, #-32)
	p2 = cmp.eq(kernel, #1)      // skip ovr if kernel == 0
#if defined(PIC) || defined(__PIC__)
	r24.l = #LO(.Lpc@GOTREL)     // Set up GOT pointer
#endif
}
{
	dalign = and(dalign, #31)
	p2 = cmp.eq(offset, #0)
	ptr_in = add(ptr_in, #-8)
	if(!p2.new) dataF8 = memd(ptr_in) // if 0 alignment dont read over end of array
}
{
	data70 = memd(ptr_in++#-8)
	if(noprolog) jump .Lnoprolog32
	align = offset
#if defined(PIC) || defined(__PIC__)
	r24 = sub(r10, r24)          // Compute GOT ptr
#endif
}
/* upto initial 7 bytes */
{
	ldata0 = valignb(dataF8, data70, align)
	ifbyte = tstbit(shift,#3)
	offset = sub(offset, star3)
}
{
	d0 = lsr(data1, #24)
	ldata0 = lsl(ldata0, shiftb)
	shiftb = and(shift, #16)
	if(ifbyte) ptr_out = add(ptr_out, #-1)
}
{
	if(ifbyte) memb(ptr_out) = d0
	ifhword = tstbit(shift,#4)
	d0 = lsr(data1, #16)
	if(ifhword.new) ptr_out = add(ptr_out, #-2)
}
{
	if(ifhword) memh(ptr_out) = d0
	ldata0 = lsl(ldata0, shiftb)
	ifword = tstbit(shift,#5)
}
{
	if(ifword) ptr_out = add(ptr_out, #-4)
	p2 = cmp.ge(offset, #0)
}
{
	if(ifword) memw(ptr_out) = data1
	if(!p2) dataF8 = data70
	if(!p2) data70 = memd(ptr_in++#-8)   // another 8 bytes
	align = offset
}
.Lnoprolog32:
{
	p3 = sp1loop0(.Ldword_loop_prolog, prolog)
	rest = sub(len, star3)   // whats left after the loop prolog8+ kernel+epilog
	ptr_out = add(ptr_out, #-8)
#if defined(PIC) || defined(__PIC__)
	r31 = add(r31, r24)          // GOT-relative jump to .Lmemcpy_return.
#endif
}
.Ldword_loop_prolog:
{
	if(p3) memd(ptr_out++#-8) = ldata0
	ldata0 = valignb(dataF8, data70, align)
	p0 = cmp.gt(rest, #8)
}
{
	dataF8 = data70
	if(p0) data70 = memd(ptr_in++#-8)
	rest = add(rest, #-8)
}:endloop0
	ptr_out = add(ptr_out, #8)
/*---------------------------------------------------*/
.Lkernel:
{
	p3 = cmp.gtu(kernel, #0)             // kernel is at least 32bytes
	if(p3.new) kernel = add(kernel, #-1) // last itn. remove edge effects
	if(p3.new) epilog = add(epilog, #32) // dealt with in last dword loop
}
	p3 = cmp.eq(kernel, #0)
{
	if(p3) jump  .Lepilog
	inc = combine(#-32, #-1)
	p1 = cmp.gt(dalign, #24)
}
{
	if(p1) jump  .Lodd_alignment
	rest = add(kernel, #0)
}
{
	loop0(.Loword_loop_25to31, kernel)
	ptr_out = add(ptr_out, #-8)
}
.falign
.Loword_loop_25to31:
{
	dcfetch(ptr_in_p_128)     // prefetch 4 lines ahead
	ptr_in_p_128kernel = vaddw(ptr_in_p_128kernel, inc)
	p3 = cmp.eq(kernel, rest)
}
{
	if(!p3) memd(ptr_out++#-8) = ldata1
	ldata1 = valignb(dataF8, data70, align)
	dataF8 = memd(ptr_in++#-8)
}
{
	memd(ptr_out++#-8) = ldata0
	ldata0 = valignb(data70, dataF8, align)
	data70 = memd(ptr_in++#-8)
}
{
	memd(ptr_out++#-8) = ldata1
	ldata1 = valignb(dataF8, data70, align)
	dataF8 = memd(ptr_in++#-8) // shut off if epilog == 0
}
{
	memd(ptr_out++#-8) = ldata0
	ldata0 = valignb(data70, dataF8, align)
	data70 = memd(ptr_in++#-8) // shut off if epilog <= 8
}:endloop0
{
	memd(ptr_out) = ldata1
	jump .Lepilog
}
.Lodd_alignment:
{
	loop0(.Loword_loop_00to24, kernel)
	ptr_out = add(ptr_out, #-8)
}
.falign
.Loword_loop_00to24:
{
	dcfetch(ptr_in_p_128)     // prefetch 4 lines ahead
	ptr_in_p_128kernel = vaddw(ptr_in_p_128kernel, inc)
}
{
	memd(ptr_out++#-8) = ldata0
	ldata0 = valignb(dataF8, data70, align)
	dataF8 = memd(ptr_in++#-8)
}
{
	memd(ptr_out++#-8) = ldata0
	ldata0 = valignb(data70, dataF8, align)
	data70 = memd(ptr_in++#-8)
}
{
	memd(ptr_out++#-8) = ldata0
	ldata0 = valignb(dataF8, data70, align)
	dataF8 = memd(ptr_in++#-8) // shut off if epilog == 0
}
{
	memd(ptr_out++#-8) = ldata0
	ldata0 = valignb(data70, dataF8, align)
	data70 = memd(ptr_in++#-8) // shut off if epilog <= 8
}:endloop0
	ptr_out = add(ptr_out, #8)
/*---------------------------------------------------*/
.Lepilog:
{
	noepilog = cmp.eq(epilog,#0)
	epilogdws = lsr(epilog, #3)
}
{
	if(noepilog) jumpr r31
	p3 = cmp.eq(epilogdws, #0)
	shift2 = asl(epilog, #3)
}
{
	shiftb = and(shift2, #32)
	ifword = tstbit(epilog,#2)
	if(p3) jump .Lepilog60
}
{
	loop0(.Ldword_loop_epilog, epilogdws)
}
.Ldword_loop_epilog:
{
	memd(ptr_out+#-8) = ldata0
	ldata0 = valignb(dataF8, data70, align)
	ptr_out = add(ptr_out, #-8)
	p3 = cmp.gt(epilog, #16)
}
{
	dataF8 = data70
	if(p3) data70 = memd(ptr_in++#-8)
	epilog = add(epilog, #-8)
}:endloop0
/* copy last 7 bytes */
.Lepilog60:
{
	if(ifword) ptr_out = add(ptr_out, #-4)
}
{
	if(ifword) memw(ptr_out) = data1
	ifhword = tstbit(epilog,#1)
	ldata0 = lsl(ldata0, shiftb)
	shiftb = and(shift2, #16)
}
{
	d0 = lsr(data1, #16)
	if(ifhword) ptr_out = add(ptr_out, #-2)
	ldata0 = lsl(ldata0, shiftb)
}
{
	if(ifhword) memh(ptr_out) = d0
	ifbyte = tstbit(epilog,#0)
	d0 = lsr(data1, #24)
	if(ifbyte.new) ptr_out = add(ptr_out, #-1)
}
{
	if(ifbyte) memb(ptr_out) = d0
	jumpr r31
}
// - do byte copy for small n -
.Lshort_bytecpy:
{
	ptr_out = add(ptr_out, #-1)
	ptr_in = add(ptr_in, #-1)
	p3 = sp1loop0(.Lbyte_copy, len)
}
.falign
.Lbyte_copy:
{
	if(p3) memb(ptr_out++#-1) = data0
	data0 = memb(ptr_in++#-1)
}:endloop0
{
	memb(ptr_out) = data0
	jumpr   r31
}
// do dword copies for aligned in, out and length
.Ldwordaligned:
{
	p3 = sp1loop0(.Ldword_copy, len8)
	ptr_out = add(ptr_out, #-8)
	ptr_in = add(ptr_in, #-8)
}
.falign
.Ldword_copy:
{
	if(p3) memd(ptr_out++#-8) = ldata0
	ldata0 = memd(ptr_in++#-8)
}:endloop0
{
	memd(ptr_out) = ldata0
	jumpr r31                         // return to function caller
}
.Lmemcpy_return:
{
	r19:18 = memd(sp+#8)              // restore r18+r19
	r17:16 = memd(sp+#0)              // restore r16+r17
}
#if defined(PIC) || defined(__PIC__)
	r24 = memw(sp+#16)
#endif
	deallocframe                      // restore r31
	jumpr r31                         // return to function caller

END(BP_SYM(_hexagon_reverse_memcpy))
libc_hidden_builtin_def (_hexagon_reverse_memcpy)

