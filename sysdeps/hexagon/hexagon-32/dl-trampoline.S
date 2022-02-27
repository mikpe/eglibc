/* PLT trampolines. Hexagon-32 version.
   Copyright (C) 2010 Free Software Foundation, Inc.
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

/* We don't want to have _mcount calls in the trampoline! */
#undef CALL_MCOUNT
#define CALL_MCOUNT

ENTRY(_dl_runtime_resolve)
   .cfi_def_cfa 30, 8
   .cfi_offset 31, -4
   .cfi_offset 30, -8
   .cfi_offset 15, -12
   .cfi_offset 14, -16
   .cfi_offset 13, -20
   .cfi_offset 12, -24
   .cfi_offset 11, -28
   .cfi_offset 10, -32
   .cfi_offset 9, -36
   .cfi_offset 8, -40
   .cfi_offset 7, -44
   .cfi_offset 6, -48
   .cfi_offset 5, -52
   .cfi_offset 4, -56
   .cfi_offset 3, -60
   .cfi_offset 2, -64
   .cfi_offset 1, -68
   .cfi_offset 0, -72
   .cfi_offset 28, -76

   /* Save all(!) clobbered registers, per new ABI. */
   allocframe(#72)
   memd (fp + #-8) = r15:14
   memd (fp + #-16) = r13:12
   memd (fp + #-24) = r11:10
   memd (fp + #-32) = r9:8
   memd (fp + #-40) = r7:6
   memd (fp + #-48) = r5:4
   memd (fp + #-56) = r3:2
   memd (fp + #-64) = r1:0
   memw (fp + #-68) = r28
   /* Set up and call _dl_fixup resolver */
   {
      r0 = r15    /* object ID */
      r1 = r14    /* Index */
      call _dl_fixup
   }
   {
      /* Save resolved target address */
      r28 = r0
      /* Restore callee arguments */
      r1:0 = memd (fp + #-64)
      r3:2 = memd (fp + #-56)
   }
   {
      r5:4 = memd (fp + #-48)
      /* Restore volatile registers */
      r7:6 = memd (fp + #-40)
   }
   {
      r9:8 = memd (fp + #-32)
      r11:10 = memd (fp + #-24)
   }
   {
      r13:12 = memd (fp + #-16)
      r15:14 = memd (fp + #-8)
   }
   {
      r28 = memw (fp + #-68)
      deallocframe
      /* Jump to the resolved address */
      jumpr r28
   }
END(_dl_runtime_resolve)

#ifndef PROF

/*
 * Profiling pseudocode:
 * -- Save all volatile regs, per ABI.
 * -- Set up args for the resolver.
 * -- Call resolver.
 * -- Restore the clobbered regs.
 * -- Call the resolved function, but return to here.
 * -- If the new framesize==-1, we are done. Return.
 * -- Save the returned value from the called func.
 * -- Save the volatile regs, again.
 * -- Set up args for _dl_call_pltexit
 * -- Call _dl_call_pltexit
 * -- Restore the returned value & volatile args
 * -- Really return.
 */
ENTRY(_dl_runtime_profile)
   .cfi_def_cfa 30, 8
   .cfi_offset 31, -4
   .cfi_offset 30, -8
   .cfi_offset 15, -12
   .cfi_offset 14, -16
   .cfi_offset 13, -20
   .cfi_offset 12, -24
   .cfi_offset 11, -28
   .cfi_offset 10, -32
   .cfi_offset 9, -36
   .cfi_offset 8, -40
   .cfi_offset 7, -44
   .cfi_offset 6, -48
   .cfi_offset 5, -52
   .cfi_offset 4, -56
   .cfi_offset 3, -60
   .cfi_offset 2, -64
   .cfi_offset 1, -68
   .cfi_offset 0, -72
   .cfi_offset 28, -84

   /* Save all(!) clobbered registers, per new ABI. */
   allocframe(#88)
   memd (fp + #-8) = r15:14
   memd (fp + #-16) = r13:12
   memd (fp + #-24) = r11:10
   memd (fp + #-32) = r9:8
   memd (fp + #-40) = r7:6
   memd (fp + #-48) = r5:4
   memd (fp + #-56) = r3:2
   memd (fp + #-64) = r1:0
   memd (fp + #-72) = r15:14  /* Again, for the pltexit */
   memw (fp + #-76) = r28
   /* Set up and call _dl_fixup resolver */
   {
      r0 = r15    /* object ID */
      r1 = r14    /* Index */
      r2 = memw (fp + #4) /* Return address */
   }
   {
      r3 = add (fp, #-64)  /* ptr to La_hexagon_32_regs */
      r4 = add (fp, #-80)  /* Returned frame size pointer */
      call _dl_profile_fixup
   }
   {
      /* Save resolved target address */
      r28 = r0
      /* Restore callee arguments */
      r1:0 = memd (fp + #-64)
      r3:2 = memd (fp + #-56)
   }
   {
      r5:4 = memd (fp + #-48)
      /* Restore volatile registers */
      r7:6 = memd (fp + #-40)
   }
   {
      r9:8 = memd (fp + #-32)
      r11:10 = memd (fp + #-24)
   }
   {
      r13:12 = memd (fp + #-16)
      r15:14 = memd (fp + #-8)
   }
   /* Call the resolved function, but return to here. */
   {
      r28 = memw (fp + #-76)
      callr r28
   }
   /* If the returned framesize is -1 or -2, then we are done. */
   {
      memd (fp + #-8) = r15:14
      r15 = memw(fp + #-80) /* returned framesize */
   }
   {
      p2 = cmp.gt(r15, #0)
      if (!p2.new) jump:nt 1f
      r15:14 = memd (fp + #-8)
   }
   deallocframe
   jumpr r31
1:
   /* Get ready to call pltexit */
   /* Save all clobbered registers, again. */
   memd (fp + #-8) = r15:14
   memd (fp + #-16) = r13:12
   memd (fp + #-24) = r11:10
   memd (fp + #-32) = r9:8
   memd (fp + #-40) = r7:6
   memd (fp + #-48) = r5:4
   memd (fp + #-56) = r3:2
   memd (fp + #-64) = r1:0

   /* Object ID, symbol index previously saved r14:15. */
   {
      r0 = memw (fp + #-68) /* Object id */
      r1 = memw (fp + #-72) /* Symbol index */
      r2 = add (fp, #-64)   /* const struct La_hexagon_32_regs * */
      r3 = add (fp, #-64)   /* struct La_hexagon_32_retval * */
   }
   call _dl_call_pltexit

   /* Restore all clobbered regs */
   {
      r1:0 = memd (fp + #-64)
      r3:2 = memd (fp + #-56)
   }
   {
      r5:4 = memd (fp + #-48)
      r7:6 = memd (fp + #-40)
   }
   {
      r9:8 = memd (fp + #-32)
      r11:10 = memd (fp + #-24)
   }
   {
      r13:12 = memd (fp + #-16)
      r15:14 = memd (fp + #-8)
   }

   /* Really return, this time. */
   deallocframe
   jumpr r31
END(_dl_runtime_profile)

#endif
