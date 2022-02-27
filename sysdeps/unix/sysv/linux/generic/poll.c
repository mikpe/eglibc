/* Poll system call, with emulation if it is not available.
   Copyright (C) 1997,1998,1999,2000,2001,2002,2006
	Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <errno.h>
#include <sys/poll.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include <kernel-features.h>

int
__poll (struct pollfd *fds, nfds_t nfds, int timeout)
{
  struct timespec timeout_ts;
  struct timespec *timeout_ts_p = NULL;

  if (timeout >= 0)
    {
      timeout_ts.tv_sec = timeout / 1000;
      timeout_ts.tv_nsec = (timeout % 1000) * 1000000;
      timeout_ts_p = &timeout_ts;
    }

  if (SINGLE_THREAD_P)
    return INLINE_SYSCALL (ppoll, 5, CHECK_N (fds, nfds), nfds,
                           timeout_ts_p, NULL, 0);

  int oldtype = LIBC_CANCEL_ASYNC ();

  int result = INLINE_SYSCALL (ppoll, 5, CHECK_N (fds, nfds), nfds,
                               timeout_ts_p, NULL, 0);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}
libc_hidden_def (__poll)
weak_alias (__poll, poll)
strong_alias (__poll, __libc_poll)
