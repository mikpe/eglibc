/* Copyright (C) 2010
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
#include <stdio.h>
#include <pthread.h>
#include <eglibc/dynconf.h>
#include <sysdep.h>

static long int conf[EGLIBC_CONF_MAX] = {
  /* EGLIBC_CONF_BUFSIZ */ BUFSIZ,
  /* EGLIBC_CONF_THREAD_STACK_MIN */ PTHREAD_STACK_MIN
};

long int
__eglibc_getconf (int name)
{
  /* Check that NAME is valid.  */
  if (__builtin_expect (name < 0 || name >= EGLIBC_CONF_MAX, 0))
    {
      __set_errno (EINVAL);
      return -1;
    }
  
  return conf[name];
}

libc_hidden_def (__eglibc_getconf)
weak_alias (__eglibc_getconf, eglibc_getconf)

int
__eglibc_setconf (int name, long int value)
{
  /* Check that NAME is valid.  */
  if (__builtin_expect (name < 0 || name >= EGLIBC_CONF_MAX, 0))
    goto error;

  switch (name)
    {
    case EGLIBC_CONF_BUFSIZ:
      if (value < 1)
	goto error;
      break;

    case EGLIBC_CONF_THREAD_STACK_MIN:
      if (value < 1)
	goto error;
      break;
    }

  conf[name] = value;
  return 0;

 error:
  return -1;
}

libc_hidden_def (__eglibc_setconf)
weak_alias (__eglibc_setconf, eglibc_setconf)

