/* Copyright (C) 2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Linas Vepstas <linasvepstas@gmail.com>, 2010.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>

/* _start entry point for Hexagon.

   The incoming stack is assumed to look like this:
        sp -> argc
           argv[0]
           ...
           NULL
           envp[0]
           ...
           NULL

   Hexagon ABI specifies that the loader will put rtld_fini in r28.
   (This is done in RTLD_START in dl-machine.h)

   All that is done here is to set up the args and call:

   #define LIBC_START_MAIN __libc_start_main and etc.
   void __libc_start_main(
            int (*main)(int, char **, char **),      // r0
            int argc,                                // r1
            char **argv,                             // r2
            void (*app_init)(int, char **, char **), // r3
            void (*app_fini)(void),                  // r4
            void (*rtld_fini)(void),                 // r5
            void *stack_end)                         // onstack

   (Note that only r0-r5 pass args, the rest are spilled to stack)

   Set GP to non-zero _SDA_BASE_ only if linker provided an SDA_BASE.
*/

#define PTR_SIZE 4

.weak _SDA_BASE_

/* Sure can't call mcount before we have a stack! */
#undef CALL_MCOUNT
#define CALL_MCOUNT

ENTRY(_start)
/* Make sure the frame register is undefined, as we must never unwind
 * past here. That is, we want a CIE with DW_CFA_undefined for this
 * frame. */ 
.cfi_undefined 29
.cfi_undefined 30
.cfi_undefined 31
	{
		R1 = memw(SP);          /* argc */
		R2 = add(SP,#PTR_SIZE); /* argv */
		R5 = REG_FINI;          /* rtld_fini */
	}
	{
		FP = #0;                /* Don't allow stack pop past here */
		SP = and (SP, #-16);    /* align stack */
	}
	{
		R6 = SP;                /* stack_end */
		memw(SP+#-8) = SP;      /* stack_end goes on stack */
		SP = add (SP, #-8);
	}

#if defined(PIC) || defined(__PIC__)

	/*
	 * If the application itself is compiled as PIC, then _start has
	 * to go through the GOT to find main(), _init(), _fini(), and sda_base.
	 */

1:	{
		R24 = pc;
		R10.H = #HI(1b@GOTOFF);
		R3.H = #HI(__libc_csu_init@GOTOFF);
		R4.H = #HI(__libc_csu_fini@GOTOFF);
	}
	{
		R10.L = #LO(1b@GOTOFF);
		R3.L = #LO(__libc_csu_init@GOTOFF);
		R4.L = #LO(__libc_csu_fini@GOTOFF);
		R8.L = #LO(_SDA_BASE_@GOTOFF);
	}
	{
		R24 = sub(R24,R10);         /* r24 now holds offset to got */
		R8.H = #HI(_SDA_BASE_@GOTOFF);
	}
	{
		R0.H = #HI(main@GOT);
		R3 = add(R3, R24);
		R4 = add(R4, R24);
		P0 = cmp.eq (R8, #0);
	}
	{
		R0.L = #LO(main@GOT);
		if (!P0) R8 = add(R8, R24);
	}
	R0 = add(R0, R24);
	R0 = memw(R0+#0) ;
	{
		GP = R8;
		call __libc_start_main@PLT;
	}

#else
	/*  non-PIC applications don't need to go through contortions to find stuff.  */
	{
		R0.L = #LO(main);
		R8.L = #LO(_SDA_BASE_);
		R7.L = #LO(__libc_start_main);
	}
	{
		R0.H = #HI(main);
		R8.H = #HI(_SDA_BASE_);
		R7.H = #HI(__libc_start_main);
	}
	{
		R3.L = #LO(__libc_csu_init);
		R4.L = #LO(__libc_csu_fini);
	}
	{
		R3.H = #HI(__libc_csu_init);
		R4.H = #HI(__libc_csu_fini);
	}
	{
		GP = R8;
		callr R7;
	}
#endif

	/* callee should never return */
2:
	brkpt;
	jump 2b;

END(_start)
