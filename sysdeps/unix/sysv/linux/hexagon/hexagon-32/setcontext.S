/* Jump to a new context -- Hexagon version.
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

ENTRY(__setcontext)
	.cfi_def_cfa 30, 8
	.cfi_offset 31, -4
	.cfi_offset 30, -8
	.cfi_offset 0, -12
	allocframe(#8)

	memw(fp+#-4) = r0

	/* Restore the signal mask */
	{
	r1 = add(r0, #UCONTEXT_SIGMASK)
	r2 = #0
	r0 = #SIG_SETMASK
	}
	call PLTJMP(__sigprocmask)

	p0 = cmp.eq(r0, #0)
	if (!p0) jump 1f

	r0 = memw(fp+#-4)

	/* Restore the callee-saved GPR's. */
	{
	r16 = memw(r0+#REG_R16)
	r17 = memw(r0+#REG_R17)
	}
	{
	r18 = memw(r0+#REG_R18)
	r19 = memw(r0+#REG_R19)
	}
	{
	r20 = memw(r0+#REG_R20)
	r21 = memw(r0+#REG_R21)
	}
	{
	r22 = memw(r0+#REG_R22)
	r23 = memw(r0+#REG_R23)
	}
	{
	r24 = memw(r0+#REG_R24)
	r25 = memw(r0+#REG_R25)
	}
	{
	r26 = memw(r0+#REG_R26)
	r27 = memw(r0+#REG_R27)
	}
	{
	r28 = memw(r0+#REG_R28)

	/* Needed for __start_context, below. */
	r8 = memw(r0+#REG_R8)
	}

	/* Switch to new stack, but not atomically, so can't use r29 */
	r9 = memw(r0+#REG_R29)

	/* Perform fake allocframe on new stack */
	{
	r10 = add(r9, #-8)

	/* deallocframe below will restore #REG_R30 */
	r6 = memw(r0+#REG_R30)
	}
	{
	memw(r10) = r6

	/* deallocframe below will restore #REG_R31 */
	r6 = memw(r0+#REG_R31)
	}
	{
	memw(r10+#4) = r6

	/* Fake out the return value */
	r0 = #0
	r1 = #0 /* Not the first time through-- swapcontext */
	}

	/* Switch to new stack, for real this time. */
	{
	fp = r10
	sp = r9
	}

	/* End FDE here, we fall into another context.  */
	cfi_endproc
	cfi_startproc

1:	/* L(error_exit): */
	deallocframe
	jumpr r31

END(__setcontext)

weak_alias (__setcontext, setcontext)


/* Trampoline for makecontext; see makecontext.c for details. */
ENTRY(__start_context)

	/* Pull the args out of the context. */
	r0 = memw(r8+#REG_R0)
	r1 = memw(r8+#REG_R1)
	r2 = memw(r8+#REG_R2)
	r3 = memw(r8+#REG_R3)
	r4 = memw(r8+#REG_R4)
	r5 = memw(r8+#REG_R5)
	r6 = memw(r8+#REG_R6)  /* func */
	r7 = memw(r8+#REG_R7)  /* uc_link */

	r17 = r7               /* copy to callee-saved */
	callr r6               /* Call func */

	p0 = cmp.eq(r17, #0)   /* If uc_link is null, then exit */
	if (p0) jump 1f;

	r0 = r17               /* uc_link in r0 */
	call PLTJMP(setcontext)

	/* NOTREACHED, crash and burn */
	r0 = #1
	memw(#40) = r0
1:
	call PLTJMP(exit)

	/* NOTREACHED, crash and burn */
	r0 = #42
	memw(#0) = r0

END(__start_context)

