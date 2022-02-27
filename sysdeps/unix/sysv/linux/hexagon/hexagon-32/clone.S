/* Wrapper around clone system call.
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
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA
   02110-1301 USA.  */

#include <sysdep.h>
#include <bp-sym.h>
#include <tcb-offsets.h>
#define _ERRNO_H        1
#include <bits/errno.h>

#define CLONE_THREAD 0x00010000
#define CLONE_VM     0x00000100

/* Flow-chart, roughly:
   allocframe for parent
   Check for bad args: child_stack == NULL || fn == NULL.
   if bad args, do syscall_error
   else
   setup clone syscall args
   call clone system call
   if (child) {
      set up stack for child
      call fn
      call _exit with result from procedure
   }
   if parent, deallocframe, return

   ----------------------------------
   This call has the C signature:

   int [r0] clone(int (*fn)(void *arg) [r0],
                  void *child_frame [r1],
                  int flags [r2],
                  void *arg [r3],  (same as tcb)
                  void *parent_tid [r4],
                  void *tcb [r5],
                  void *child_tid [memw(r30+#8)]);

   By contrast, the syscall has the C signature:
   int [r0] clone(int flags [r0],
                  void *child_frame [r1],
                  void *parent_tid [r2],
                  void *child_tid [r3],
                  void *tcb [r4]);

 */

ENTRY (BP_SYM (__clone))
	.cfi_def_cfa 30, 8
	.cfi_offset 31, -4
	.cfi_offset 30, -8
	/* Check for NULL function, or child-stack */
	{
		/* Set up the stack frame for the parent */
		/* Parent needs only 8, for salting away errno */
		allocframe(#8)

		p0 = cmp.eq(r0, #0)
		p1 = cmp.eq(r1, #0)
		if p0.new jump:nt .Lbadargs
	}
	if p1 jump .Lbadargs

	{
		/* Save fn ptrs for child */
		r11 = r0  /* fn    in r11 */
		r10 = r3  /* arg aka tcb in r10 */
		r9  = r2  /* flags in r9  */
	}
	{
		/* Set up syscall args */
		r0 = r2             /* flags */
		r2 = r4             /* parent_tid */
		r4 = r3             /* arg/tcb in r4 */
	}
	.cfi_offset 3, -16
	r3 = memw(r30+#8)      /* child_tid */

	/* Make the syscall */
	DO_CALL(clone)

	/* Check for child */
	{
		p0 = cmp.eq(r0, #0)
		if p0.new jump:nt .Lchild
		r6 = #-(MAX_KERNEL_ERRNO+1)
	}

	/* We are parent; check for error. */
	{
		p0 = cmp.gtu(r0, r6)
		if p0.new jump:nt .Lsyserr
	}

	deallocframe
	jumpr r31
	cfi_endproc

.Lchild:
	cfi_startproc
	.cfi_def_cfa 30, 8
	.cfi_offset 31, -4
	.cfi_offset 30, -8
	/* Set up childs stack, zero out saved LR/FP */
	{
		allocframe(#0)
		r1 = #0
	}
	memw(r30 + #4) = r1     /* zero out saved LR */
	memw(r30) = r1          /* zero out saved FP */

#ifdef RESET_PID
	/* The below implements the following:
	 * if (0 == (flags & CLONE_THREAD)) {
	 *    int newpid = -1;
	 *    if (0 == (flags & CLONE_VM)) newpid = getpid();
	 *    tcb->tid = newpid;
	 *    tcb->pid = newpid;
	 * }
	 */
	{
		r2.l = #(CLONE_THREAD & 0xffff)
		r3.l = #(CLONE_VM & 0xffff)
	}
	{
		r2.h = #(CLONE_THREAD >> 16)
		r3.h = #(CLONE_VM >> 16)
	}
	{
		r2 = and(r9, r2)
		r3 = and(r9, r3)
	}
	{
		p2 = cmp.eq(r2, #0)
		p3 = cmp.eq(r3, #0)
	}
	if !p2 jump .Lnochange
	{
		r0 = #-1
		if !p3 jump .Lsetpid
	}

	DO_CALL(getpid)
.Lsetpid:
	r2 = ugp
	memw (r2 + #TID) = r0
	memw (r2 + #PID) = r0
.Lnochange:
#endif
	{
		r0 = r10
		callr r11
	}

	/* Call _exit with the returned value from subr */
	call PLTJMP(_exit)

.Lsyserr:
	// todo: Perhaps we should be calling __syscall_error here?
	// See sysdeps/unix/sysv/linux/hexagon/hexagon-32/sysdep.h
	// I don't see that it matters ...
	memw(r30+#-4) = r0

	/* clone syscall returned an error */
	call PLTJMP(__errno_location)

	/* get -ERRVAL and store it in errno */
	r1 = memw(r30+#-4)
	r1 = sub(#0, r1)
	memw(R0) = r1
	r0 = #-1

	deallocframe
	jumpr r31

.Lbadargs:
	r0 = #-EINVAL
	jump .Lsyserr

END (BP_SYM (__clone))

weak_alias (BP_SYM (__clone), BP_SYM (clone))
