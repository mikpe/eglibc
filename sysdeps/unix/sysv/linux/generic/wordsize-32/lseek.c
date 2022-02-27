/* Long-long seek operation.
   Copyright (C) 1996-2000,2002,2003 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <sys/types.h>

#include <sysdep.h>
#include <sys/syscall.h>

off_t
__lseek (int fd, off_t offset, int whence)
{
  loff_t retval;

  return (off_t) (INLINE_SYSCALL (_llseek, 5, fd, (off_t) (offset >> 31),
                                  (off_t) offset,
                                  __ptrvalue (&retval), whence) ?: retval);
}
libc_hidden_def (__lseek)
weak_alias (__lseek, lseek)
strong_alias (__lseek, __libc_lseek)
