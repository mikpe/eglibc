/* Hexagon-specific implementation of profiling support.
   Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Linas Vepstas <linasvepstas@gmail.com>, 2011

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
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA
   02110-1301 USA.  */

#include <sysdep.h>

/* Avoid recursive call to self! */
#undef CALL_MCOUNT
#define CALL_MCOUNT

/* At this time, the compiler does not provide use with any special
   profiling support; on the other hand, it is liable to drop in a
   call to mcount just about anywhere. Thus, we have to be sure to
   avoid clobbering the volatile regs.
 */
ENTRY(_mcount)
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
	allocframe(#64)

	/* Avoid clobbering volatile regs */
	memd(sp) = r1:0
	memd(sp+#8) = r3:2
	memd(sp+#16) = r5:4
	memd(sp+#24) = r7:6
	memd(sp+#32) = r9:8
	memd(sp+#40) = r11:10
	memd(sp+#48) = r13:12
	memd(sp+#56) = r15:14

	/* The return address is in the previous frame */
	r0 = memw(r30)
	r0 = memw(r0 + #4)

	/* The called address is in this frame. */
	r1 = memw(r30 + #4)

	call PLTJMP(__mcount_internal)

	/* Restore the volatile regs */
	r1:0 = memd(sp)
	r3:2 = memd(sp+#8)
	r5:4= memd(sp+#16)
	r7:6 = memd(sp+#24)
	r9:8 = memd(sp+#32)
	r11:10 = memd(sp+#40)
	r13:12 = memd(sp+#48)
	r15:14 = memd(sp+#56)

	deallocframe
	jumpr r31

END(_mcount)

weak_alias (_mcount, mcount)

