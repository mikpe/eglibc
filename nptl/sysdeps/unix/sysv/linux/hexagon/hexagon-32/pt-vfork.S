
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
#define _ERRNO_H	1
#include <bits/errno.h>
#include <kernel-features.h>
#include <tcb-offsets.h>
#include <asm/unistd.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)

	// Flip sign on the PID, restore after syscall
	r2 = ugp
	r1 = memw(r2 + #PID)
	r1 = neg(r1)
	memw(r2 + #PID) = r1;

	/* flags.
	   linux/sched.h:		CLONE_VM	0x00000100
	   linux/sched.h:		CLONE_VFORK	0x00004000
	   asm-generic/signal.h:	SIGCHLD		17
	*/	
	r0 = #0x00004111

	/* child_frame.  */
	r1 = #0

	/* parent_tid.  Used only if CLONE_PARENT_SETTID flag is set.   */
	r2 = #0
	
	/* child_tid.  Used only if CLONE_CHILD_SETTID flag is set.  */
	r3 = #0

	/* tcb.  Used only if CLONE_SETTLS flag is set.  */
	r4 = #0

	DO_CALL (clone)

	p2 = cmp.eq(r0, #0)
	if p2 jump 1f

	/* Parent.  */

	/* Restore PID.  */
	r2 = ugp
	r1 = memw(r2 + #PID)
	r1 = neg(r1)
	memw(r2 + #PID) = r1;

1:
	PSEUDO_RET

PSEUDO_END (__vfork)

weak_alias (__vfork, vfork)
