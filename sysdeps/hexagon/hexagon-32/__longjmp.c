/* Copyright (C) 2010 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <setjmp.h>
#include <bits/setjmp.h>
#include <jmpbuf-offsets.h>

/* Jump to the position specified by ENV, causing the
   setjmp call there to return VAL, or 1 if VAL is 0.  */
void
__longjmp (__jmp_buf env, int val)
{
	if (0 == val) val = 1;

#ifdef CHECK_SP
# ifdef PTR_DEMANGLE
  /* XXX pter mangling not done */
  /* register uintptr_t gaurd = THREAD_GET_POINTER_GUARD (); */
  uintptr_t gaurd = 0;
  CHECK_SP (env, gaurd);
# else
  CHECK_SP (env, 0);
# endif
#endif

  /* Restore the callee-saved regs r16-r31 from jmp_buf
   * defined in bits/setjmp.h */
  __asm__(
    "{ r17:16 = memd(%0+#0);  r19:18 = memd(%0+#8);  }"
    "{ r21:20 = memd(%0+#16); r23:22 = memd(%0+#24); }"
    "{ r25:24 = memd(%0+#32); r27:26 = memd(%0+#40); }"
    "{ r29:28 = memd(%0+#48); r31:30 = memd(%0+#56); r0 = %1; }"
    "jumpr r31"
  : : "r"(env), "r"(val) :
  "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
  "r25", "r26", "r27", "r28", "r29", "r30", "r31");

  /* Avoid `volatile function does return' warnings.  */
  for (;;);
}
