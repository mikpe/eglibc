/* Copyright (C) 2001,2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Linas Vepstas <linasvepstas@gmail.com>

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

#include <ucontext.h>

struct crv
  {
    long retval;         /* r0 */
    long first_return;   /* r1 */
  };

extern struct crv __getcontext (ucontext_t *__ucp) __THROW;
extern int __setcontext (__const ucontext_t *__ucp) __THROW;

int
__swapcontext (ucontext_t *oucp, const ucontext_t *ucp)
{
  struct crv rv = __getcontext (oucp);

  /* If getcontext failed, fail out */
  if (rv.retval)
    return rv.retval;

  if (rv.first_return)
    __setcontext (ucp);

  /* not reached */
  return 0;
}

weak_alias (__swapcontext, swapcontext)
