/* Copyright (C) 2005, 2006, 2007, 2009 Free Software Foundation, Inc.
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

/* Ho hum, if fxstatat == fxstatat64 we must get rid of the prototype or gcc
   will complain since they don't strictly match.  */
#define __fxstatat64 __fxstatat64_disable

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <kernel_stat.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>
#include <kernel-features.h>

#ifndef __NR_newfstatat
#define __NR_newfstatat __NR_fstatat64
#endif

/* Get information about the file NAME in BUF.  */
int
__fxstatat (int vers, int fd, const char *file, struct stat *st, int flag)
{
  if (vers == _STAT_VER_KERNEL)
    return INLINE_SYSCALL (newfstatat, 4, fd, file, st, flag);

  errno = EINVAL;
  return -1;
}
libc_hidden_def (__fxstatat)
# undef __fxstatat64
strong_alias (__fxstatat, __fxstatat64);
hidden_ver (__fxstatat, __fxstatat64);
