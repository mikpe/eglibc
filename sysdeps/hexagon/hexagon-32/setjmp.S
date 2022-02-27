/*
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
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _ASM
#define _ASM
#endif

#include <sysdep.h>

ENTRY (__sigsetjmp)

	/* Use r5 as the call address */
	{
#if defined(PIC) || defined(__PIC__)
		r5.L = #LO(__sigjmp_save@GOTOFF);
#else
		r5.L = #LO(__sigjmp_save);
#endif
		memd(r0+#0)  = r17:16;
	}
	{
#if defined(PIC) || defined(__PIC__)
		r5.H = #HI(__sigjmp_save@GOTOFF);
#else
		r5.H = #HI(__sigjmp_save);
#endif
		memd(r0+#8)  = r19:18;
	}
1:
	{
#if defined(PIC) || defined(__PIC__)
		r4 = pc;
		r3.h = #hi (1b@GOTOFF);
#endif
		memd(r0+#16) = r21:20;
	}
	{
#if defined(PIC) || defined(__PIC__)
		r3.l = #lo (1b@GOTOFF);
#endif
		memd(r0+#24) = r23:22;
	}
	{
#if defined(PIC) || defined(__PIC__)
		r3 = sub (r4, r3);
#endif
		memd(r0+#32) = r25:24;
	}
	{
#if defined(PIC) || defined(__PIC__)
		r5 = add(r5, r3);
#endif
		memd(r0+#40) = r27:26;
	}

	memd(r0+#48) = r29:28;
	memd(r0+#56) = r31:30;

	/* jump __sigjmp_save; */
	jumpr r5;

END (__sigsetjmp)

strong_alias(__sigsetjmp, __GI___sigsetjmp)

weak_extern(_setjmp)
weak_extern(setjmp)
