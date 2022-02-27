/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Pavel Potoplyak <pavelp@quicinc.com>, 2011.

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
#include <tls.h>

#ifndef __ASSEMBLER__
# include <nptl/pthreadP.h>

# define RTLD_SINGLE_THREAD_P \
         __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
                          header.multiple_threads) == 0, 1)

# if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt
#  define SINGLE_THREAD_P \
          __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
                           header.multiple_threads) == 0, 1)
# else
#  define SINGLE_THREAD_P (1)
#  define NO_CANCELLATION 1
# endif
#else

# if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

#  undef PSEUDO
#  define PSEUDO(name, syscall_name, args)                       \
    ENTRY (name)                                                 \
    r7 = ugp;                                                    \
    r8 = memw (r7 + #MULTIPLE_THREADS);                          \
    {                                                            \
        p1 = cmp.eq (r8, #0);                                    \
        if (!p1.new) jump:nt .Lpseudo_cancel;                    \
    }                                                            \
    .type __##syscall_name##_nocancel , @function;               \
    .globl __##syscall_name##_nocancel;                          \
    __##syscall_name##_nocancel:                                 \
    DO_CALL (syscall_name);                                      \
    PSEUDO_RET                                                   \
    END(__##syscall_name##_nocancel);                            \
.Lpseudo_cancel:                                                 \
    cfi_startproc;                                               \
    .cfi_def_cfa 30, 8;                                          \
    .cfi_offset 31, -4;                                          \
    .cfi_offset 30, -8;                                          \
    allocframe(#32);                                             \
    CALL_ENABLE_##args;                                          \
    DO_CALL (syscall_name);                                      \
    {                                                            \
       memw(r30 + #-32) = r0; /* store syscall return value */   \
       r0 = memw(r30 + #-28);                                    \
       CDISABLE;                                                 \
    }                                                            \
    {                                                            \
       r0 = memw(r30 + #-32);                                    \
       deallocframe;                                             \
    }                                                            \
    SYSCALL_ERROR_HANDLER

/* Packetized calls to save and restore the clobbered call args. */
/* Loads can take slots 0,1, stores slot 0 only, jmp slot 2 */
#  define CALL_ENABLE_0  CENABLE \
                         memw(r30 + #-28) = r0;

#  define CALL_ENABLE_1  .cfi_offset 0, -12; \
                         { memw(r30 + #-4) = r0; CENABLE } \
                         { r0 = memw(r30 + #-4); memw(r30 + #-28) = r0; }

#  define CALL_ENABLE_2  .cfi_offset 0, -16; \
                         .cfi_offset 1, -12; \
                         memd(r30 + #-8) = r1:0; \
                         CENABLE \
                         memw(r30 + #-28) = r0; \
                         r1:0 = memd(r30 + #-8);

#  define CALL_ENABLE_3  .cfi_offset 2, -20; \
                         memw(r30 + #-12) = r2; \
                         CALL_ENABLE_2 \
                         r2 = memw(r30 + #-12);

#  define CALL_ENABLE_4  .cfi_offset 2, -24; \
                         .cfi_offset 3, -20; \
                         memd(r30 + #-16) = r3:2; \
                         CALL_ENABLE_2 \
                         r3:2 = memd(r30 + #-16);

#  define CALL_ENABLE_5  .cfi_offset 4, -28; \
                         memw(r30 + #-20) = r4; \
                         CALL_ENABLE_4 \
                         r4 = memw(r30 + #-20);

#  define CALL_ENABLE_6  .cfi_offset 4, -32; \
                         .cfi_offset 5, -28; \
                         memd(r30 + #-24) = r5:4; \
                         CALL_ENABLE_4 \
                         r5:4 = memd(r30 + #-24);

#  ifdef IS_IN_libpthread
#   define CENABLE    call PLTJMP(__pthread_enable_asynccancel);
#   define CDISABLE   call PLTJMP(__pthread_disable_asynccancel);
#   define __local_multiple_threads __pthread_multiple_threads
#  elif !defined NOT_IN_libc
#   define CENABLE    call PLTJMP(__libc_enable_asynccancel);
#   define CDISABLE   call PLTJMP(__libc_disable_asynccancel);
#   define __local_multiple_threads __libc_multiple_threads
#  elif defined IS_IN_librt
#   define CENABLE    call PLTJMP(__librt_enable_asynccancel);
#   define CDISABLE   call PLTJMP(__librt_disable_asynccancel);
#  else
#   error Unsupported library
#  endif

# endif
#endif
