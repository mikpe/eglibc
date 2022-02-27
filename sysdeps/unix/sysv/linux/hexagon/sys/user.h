/* Copyright (C) 1998, 1999, 2000, 2001, 2011 Free Software Foundation, Inc.
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

#ifndef _SYS_USER_H
#define _SYS_USER_H	1

#include <asm/user.h>

/* The whole purpose of this file is for GDB and GDB only.  Don't read
   too much into it.  Don't use it for anything other than GDB unless
   you know what you are doing.  */

struct user_fpregs
{
};

struct user
{
  struct user_regs_struct regs; /* General registers */

  unsigned long int u_tsize;    /* Text segment size (pages). */
  unsigned long int u_dsize;    /* Data segment size (pages). */
  unsigned long int u_ssize;    /* Stack segment size (pages). */

  unsigned long start_code;     /* Starting virtual address of text. */
  unsigned long start_data;     /* Starting virtual address of data. */
  unsigned long start_stack;    /* Starting virtual address of stack. */

  long int signal;              /* Signal that caused the core dump. */
  struct user_regs *u_ar0;      /* help gdb to find the general registers. */

  unsigned long magic;          /* Uniquely identify a core file */
  char u_comm[32];              /* User command that was responsible */
  struct user_fpregs u_fp;      /* Floating point registers */
  struct user_fpregs *u_fp0;    /* Help gdb to find the FP registers. */
};

#endif  /* sys/user.h */
