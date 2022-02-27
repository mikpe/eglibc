
/* Private macros for accessing __jmp_buf contents.  Hexagon version.
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


/* GPRS r16 through r31 are saved */
#define GPR_BASE 16

/* Stack ptr is r29, frame ptr is r30, link reg is r31 */
#define JB_SP (29-GPR_BASE)
#define JB_FP (30-GPR_BASE)
#define JB_LR (31-GPR_BASE)
