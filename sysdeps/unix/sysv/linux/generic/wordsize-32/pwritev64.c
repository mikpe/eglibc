/* Copyright (C) 1997,1998,1999,2000,2002,2003,2006
	Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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
#include <endian.h>
#include <unistd.h>
#include <sys/uio.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include <kernel-features.h>

static ssize_t
do_pwritev64 (int fd, const struct iovec *vector, int count, off64_t offset)
{
  return INLINE_SYSCALL (pwritev, __ALIGNMENT_COUNT(5,6), fd,
                         vector, count, __ALIGNMENT_ARG
                         __LONG_LONG_PAIR ((off_t) (offset >> 32),
                                           (off_t) (offset & 0xffffffff)));
}


ssize_t
__libc_pwritev64 (int fd, const struct iovec *vector, int count,
                  off64_t offset)
{
  if (SINGLE_THREAD_P)
    return do_pwritev64 (fd, vector, count, offset);

  int oldtype = LIBC_CANCEL_ASYNC ();

  ssize_t result = do_pwritev64 (fd, vector, count, offset);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}

strong_alias (__libc_pwritev64, pwritev64)
weak_alias (__libc_pwritev64, __pwritev64)
