/* Create a context -- Hexagon version
   Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).

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

#include <libintl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

/* This implementation can handle any ARGC value but only
   normal integer type parameters. Parameters of type float,
   double, complex and structure with sizes 0, 2, 4 or 8
   won't work.
   makecontext sets up a stack and the registers for the
   user context. The stack looks like this:

           size                         offset
               +-----------------------+
           n*4 | overflow args         |  0
               +-----------------------+
             4 | Saved LR              |
       fp->  4 | Saved FP              |
               +-----------------------+

   The registers are set up like this:
     r0-r5 : parameters 0 to 5
     r6    : (*func) pointer
     r7    : uc_link from ucontext structure
     r8    : ucontext itself
     r29   : stack pointer.
     r30   : frame pointer.
     r31   : return address to uc_link trampoline

   The trampoline looks like this:
     callr  r16        save pc to r31, branch to *func.
     r0 = r17          load r0  with uc_link
     callr setcontext  branch to setcontext
 */

void
__makecontext (ucontext_t *ucp, void (*func) (void), int argc, ...)
{
  extern void __start_context (void);
  unsigned long stack_top, stack_bottom;
  unsigned long *sp, *fp, i;
  va_list ap;

  /* double-word aligned stack boundries */
  stack_bottom = (unsigned long) ucp->uc_stack.ss_sp;
  stack_bottom = (stack_bottom + 7) & -8L;
  stack_top = stack_bottom + ucp->uc_stack.ss_size;
  stack_top &= -8L;

  /* stack grows down from top */
  fp = (unsigned long *) ((stack_top - 8) & -8L);

  /* Make room for args on stack. */
  sp = fp;
  if (6 < argc)
    {
      sp -= (argc - 6);
      sp = (unsigned long *) (((unsigned long) sp) & -8L);
    }

  *fp = ucp->uc_mcontext.sc_regs.r30;
  *(fp+1) = (unsigned long) __start_context;

  /* Set the return address to trampoline.  */
  ucp->uc_mcontext.sc_regs.r29 = (unsigned long) sp;
  ucp->uc_mcontext.sc_regs.r30 = (unsigned long) fp;
  ucp->uc_mcontext.sc_regs.r31 = (unsigned long) __start_context;

  /* Set register parameters.  */
  va_start (ap, argc);
  if (argc)
    ucp->uc_mcontext.sc_regs.r0 = va_arg (ap, unsigned long);
  if (1 < argc)
    ucp->uc_mcontext.sc_regs.r1 = va_arg (ap, unsigned long);
  if (2 < argc)
    ucp->uc_mcontext.sc_regs.r2 = va_arg (ap, unsigned long);
  if (3 < argc)
    ucp->uc_mcontext.sc_regs.r3 = va_arg (ap, unsigned long);
  if (4 < argc)
    ucp->uc_mcontext.sc_regs.r4 = va_arg (ap, unsigned long);
  if (5 < argc)
    ucp->uc_mcontext.sc_regs.r5 = va_arg (ap, unsigned long);

  /* The remaining arguments go to the overflow area.  */
  if (6 < argc)
    for (i=6; i<argc; i++)
      *(sp+i-6) = va_arg (ap, unsigned long);

  va_end (ap);

  /* Pass (*func) to __start_context in r6.  */
  ucp->uc_mcontext.sc_regs.r6 = (unsigned long) func;

  /* Pass ucp->uc_link to __start_context in r7.  */
  ucp->uc_mcontext.sc_regs.r7 = (unsigned long) ucp->uc_link;

  /* Pass ucp itself in r8, so that we can find r0-r7.  */
  ucp->uc_mcontext.sc_regs.r8 = (unsigned long) ucp;
}

weak_alias (__makecontext, makecontext)
