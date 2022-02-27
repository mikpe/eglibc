
/* Definition for thread-local data handling.  NPTL/Hexagon version.
   Copyright (C) 2010 Free Software Foundation, Inc.
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

#ifndef _TLS_H
#define _TLS_H	1

# include <dl-sysdep.h>

#ifndef __ASSEMBLER__
# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>

/* Type for the dtv.  */
typedef union dtv
{
  size_t counter;
  struct
  {
    void *val;
    bool is_static;
  } pointer;
} dtv_t;

#else /* __ASSEMBLER__ */
# include <tcb-offsets.h>
#endif /* __ASSEMBLER__ */

#ifndef __ASSEMBLER__

/* Get system call information.  */
# include <sysdep.h>

/* The TP points to the start of the TCB (thread control block)
   variant II of the Drepper v0.20 "ELF handling for TLS" document
   The TCB can have any size, and the memory following the address the
   thread pointer points to is unspecified.  Allocate the TCB there.
 */
# define TLS_TCB_AT_TP	1

/* We use the multiple_threads field in the pthread struct */
# define TLS_MULTIPLE_THREADS_IN_TCB	1

typedef struct
{
  uintptr_t pointer_guard;
  dtv_t *dtv;
  void *tcb; /* Pointer to the TCB.  Not necessarily the
                thread descriptor used by libpthread.  */
  void *self;     /* Pointer to the thread descriptor.  */
  int multiple_threads;
  int gscope_flag;
} tcbhead_t;

/* Get the thread descriptor definition.  */
# include <nptl/descr.h>

/* This is the size of the initial TCB.  */
# define TLS_INIT_TCB_SIZE	sizeof (struct pthread)

/* Alignment requirements for the initial TCB.  */
# define TLS_INIT_TCB_ALIGN	__alignof__ (struct pthread)

/* This is the size of the TCB.  */
# define TLS_TCB_SIZE      sizeof (struct pthread)

/* Alignment requirements for the TCB.  */
# define TLS_TCB_ALIGN		__alignof__ (struct pthread)

/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contains the length.  */
# define INSTALL_DTV(descr, dtvp) \
  ((tcbhead_t *) (descr))->dtv = (dtvp) + 1

/* Install new dtv for current thread.  */
# define INSTALL_NEW_DTV(dtvp) \
  ({ struct pthread *__pd = THREAD_SELF;  \
     THREAD_SETMEM (__pd, header.dtv, (dtvp)); })

/* Return dtv of given thread descriptor.  */
/* Must not be null, even for emutls, since _dl_allocate_tls_init writes it */
# define GET_DTV(descr)  (((tcbhead_t *) (descr))->dtv)

/* Code to initially initialize the thread pointer.  */
# define TLS_INIT_TP(tcbp, secondcall) \
    ({ asm("ugp = %0;" : : "r"(tcbp)); NULL; })

/* Return the thread descriptor for the current thread.  */
# define THREAD_SELF \
    ({ struct pthread *__thread_self; \
       asm("%0 = ugp;" : "=r"(__thread_self)); \
       __thread_self;})

/* Return the address of the dtv for the current thread.  */
# define THREAD_DTV() (((tcbhead_t *) THREAD_SELF)->dtv)

/* By convention, (but not formally discussed in the ABI??)
 * the compiler will copy ugp into r25, and will deal with
 * handling r25 as a callee-saved reg. */
#define THREAD_REG r25
#define THREAD_REG_NUM 25

/* Magic for libthread_db to know how to do THREAD_SELF.  */
# define DB_THREAD_SELF  REGISTER (32, 32, THREAD_REG_NUM * 4, 0)

/* Read member of the thread descriptor directly.  */
# define THREAD_GETMEM(descr, member) ((descr)->member)

/* Same as THREAD_GETMEM, but the member offset can be non-constant.  */
# define THREAD_GETMEM_NC(descr, member, idx) ((descr)->member[idx])

/* Set member of the thread descriptor directly.  */
# define THREAD_SETMEM(descr, member, value) ((descr)->member = (value))

/* Same as THREAD_SETMEM, but the member offset can be non-constant.  */
# define THREAD_SETMEM_NC(descr, member, idx, value) \
    ((descr)->member[idx] = (value))

/* Get/set the pointer guard field in TCB head.  */
# define THREAD_GET_POINTER_GUARD() \
  THREAD_GETMEM (THREAD_SELF, header.pointer_guard)
# define THREAD_SET_POINTER_GUARD(value) \
  THREAD_SETMEM (THREAD_SELF, header.pointer_guard, value)
# define THREAD_COPY_POINTER_GUARD(descr) \
  ((descr)->header.pointer_guard = THREAD_GET_POINTER_GUARD ())

/* Get and set the global scope generation counter in struct pthread.  */
# define THREAD_GSCOPE_FLAG_UNUSED 0
# define THREAD_GSCOPE_FLAG_USED   1
# define THREAD_GSCOPE_FLAG_WAIT   2
# define THREAD_GSCOPE_RESET_FLAG() \
  do									     \
    { int __res								     \
	= atomic_exchange_rel (&THREAD_SELF->header.gscope_flag,	     \
			       THREAD_GSCOPE_FLAG_UNUSED);		     \
      if (__res == THREAD_GSCOPE_FLAG_WAIT)				     \
	lll_futex_wake (&THREAD_SELF->header.gscope_flag, 1, LLL_PRIVATE);   \
    }									     \
  while (0)
# define THREAD_GSCOPE_SET_FLAG() \
  do									     \
    {									     \
      THREAD_SELF->header.gscope_flag = THREAD_GSCOPE_FLAG_USED;	     \
      atomic_write_barrier ();						     \
    }									     \
  while (0)
# define THREAD_GSCOPE_WAIT() \
  GL(dl_wait_lookup_done) ()

#endif /* __ASSEMBLER__ */

#endif	/* tls.h */
