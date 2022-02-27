/* Save the current context -- Hexagon version
   Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Linas Vepstas <linasvepstas@gmail.com>.

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
#include <features.h>
#include "ucontext_i.h"

/*  __getcontext (ucontext_t *ucp)

  Saves the machine context in UCP such that when it is activated,
  it appears as if __getcontext() returned again.  The only difference
  is that on a first return, r0 contains 1 and on a subsequent
  return, it contains 0.

  This implementation in intended to be used for *synchronous* context
  switches only.  Therefore, it does not have to save anything
  other than the PRESERVED state.  */

ENTRY(__getcontext)
	.cfi_def_cfa 30, 8
	.cfi_offset 31, -4
	.cfi_offset 30, -8
	.cfi_offset 0, -12
	allocframe(#8)

	/* Save the GPR's */
	memw(r0+#REG_R1) = r1
	memw(r0+#REG_R2) = r2
	memw(r0+#REG_R3) = r3
	memw(r0+#REG_R4) = r4
	memw(r0+#REG_R5) = r5
	memw(r0+#REG_R6) = r6
	memw(r0+#REG_R7) = r7
	memw(r0+#REG_R8) = r8
	memw(r0+#REG_R9) = r9

	memw(r0+#REG_R10) = r10
	memw(r0+#REG_R11) = r11
	memw(r0+#REG_R12) = r12
	memw(r0+#REG_R13) = r13
	memw(r0+#REG_R14) = r14
	memw(r0+#REG_R15) = r15
	memw(r0+#REG_R16) = r16
	memw(r0+#REG_R17) = r17
	memw(r0+#REG_R18) = r18
	memw(r0+#REG_R19) = r19

	memw(r0+#REG_R20) = r20
	memw(r0+#REG_R21) = r21
	memw(r0+#REG_R22) = r22
	memw(r0+#REG_R23) = r23
	memw(r0+#REG_R24) = r24
	memw(r0+#REG_R25) = r25
	memw(r0+#REG_R26) = r26
	memw(r0+#REG_R27) = r27
	memw(r0+#REG_R28) = r28

	/* User control registers */
	r6 = c0
	memw(r0+#REG_SA0) = r6
	r6 = c1
	memw(r0+#REG_LC0) = r6
	r6 = c2
	memw(r0+#REG_SA1) = r6
	r6 = c3
	memw(r0+#REG_LC1) = r6
	r6 = c6
	memw(r0+#REG_M0 ) = r6
	r6 = c7
	memw(r0+#REG_M1) = r6
	r6 = c8
	memw(r0+#REG_USR) = r6
	r6 = c4
	memw(r0+#REG_P3_0) = r6
	r6 = #0
	memw(r0+#REG_GP) = r6
	r6 = c10
	memw(r0+#REG_UGP) = r6

	r6 = #0
	memw(r0+#REG_CAUSE) = r6
	memw(r0+#REG_BADVA) = r6
	memw(r0+#REG_PAD1) = r6
	memw(r0+#REG_PAD2) = r6
	memw(r0+#REG_PAD3) = r6

	/* Set the callers link reg, and the ucontext lr and pc,
	   to the callers return address. */
	// XXX neeed some kind of cfi_offset here I think ???
	r6 = memw(fp)
	memw(r0+#REG_R30) = r6
	r6 = memw(fp+#4)
	memw(r0+#REG_R31) = r6
	memw(r0+#REG_PC) = r6

	/* Undo the allocframe for the stack ptr before saving it...*/
	r6 = add(fp,#8)
	memw(r0+#REG_R29) = r6

	/* sigprocmask (SIG_BLOCK, NULL, &sc->sc_mask). */
	memw(fp+#-4) = r0
	r2 = add (r0, #UCONTEXT_SIGMASK)
	r0 = #SIG_BLOCK
	r1 = #0
	call PLTJMP(__sigprocmask)
	r0 = memw(fp+#-4)

	/* Return value of setcontext is success so store a zero in r0 */
	r6 = #0
	memw(r0+#REG_R0) = r6

	/* Return value of success */
	r0 = #0
	r1 = #1  /* first time through, needed for swapcontext */

	deallocframe
	jumpr r31
END(__getcontext)

weak_alias (__getcontext, getcontext)
