/* Generic assembly macros for Hexagon
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
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA
   02110-1301 USA.  */

#include <sysdeps/generic/sysdep.h>
#include <bits/wordsize.h>

#ifdef __ELF__
#ifdef __ASSEMBLER__

#define ASM_TYPE_DIRECTIVE(name,typearg) .type name, @##typearg;
#define ASM_SIZE_DIRECTIVE(name) .size name,.-name

/* If compiled for profiling, call `_mcount' at the start of each
   function.  Be sure not to clobber r31! */
#ifdef   PROF
# define CALL_MCOUNT                   \
  sp = add (sp, #-8);                  \
  memw(sp) = r31;                      \
  call PLTJMP(_mcount);                \
  r31 = memw(sp);                      \
  sp = add (sp, #8);
#else
# define CALL_MCOUNT  /* Nothing */
#endif /* PROF */

#define ENTRY(name)                    \
  .text ;                              \
  .p2align 2 ;                         \
  .globl name ;                        \
  .type name, @function ;              \
name##:                                \
  cfi_startproc ;                      \
  CALL_MCOUNT

#undef  END
#define END(name)                      \
  cfi_endproc ;                        \
  ASM_SIZE_DIRECTIVE(name)

#ifdef SHARED
#define PLTJMP(_x)   _x##@PLT
#else
#define PLTJMP(_x)   _x
#endif

/* Register in which rtld_fini is passed */
/* This value is fixed by the hexagon ABI */
#define REG_FINI r28

#endif /* __ELF__ */
#endif   /* __ASSEMBLER__ */
