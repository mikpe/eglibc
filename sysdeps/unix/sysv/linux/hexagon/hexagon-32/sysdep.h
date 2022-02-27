/* System calls.  Hexagon version.
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


#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H

#ifdef  __cplusplus
# define __BEGIN_DECLS  extern "C" {
# define __END_DECLS  }
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif

#include <sysdeps/unix/hexagon/sysdep.h>
#include <sysdeps/unix/sysv/linux/generic/sysdep.h>
#include <dl-sysdep.h>
#include <sysdeps/generic/sysdep.h>

#define ARG_TYPE long

#define MAX_KERNEL_ERRNO 4095

#undef SYS_ify
#define SYS_ify(syscall_name)  (__NR_##syscall_name)

#ifdef __ASSEMBLER__

/*
 * glibc makes a giant assembly file full of system
 * calls which use these PSEUDO macros.
 *
 * There are three versions of 'pseudo' syscalls:
 *
 * 1.  Normal syscalls with all the error checking and
 *     handling.
 * 2.  Syscalls that don't do any error checking
 * 3.  Syscalls that negate the syscall result (posix_madvise?);
 *     errno is not set
 *
 * Each has the main call macro, a return macro, and an end
 * macro.
 */

/*
 * Assembly version of the system call.  If we can assume that the registers
 * already hold the arguments in the right place, then all we need to do is put
 * the system call number in R6 and hit the trap...  This macro does not do
 * any error handling.
 */
#define DO_CALL(syscall_name)          \
    r6 = #(SYS_ify(syscall_name));     \
    trap0(#1);

/*
 * Pseudo-system call.  Errno setting is done by the
 * SYSCALL_ERROR_HANDLER macro.
 */
#define PSEUDO(name, syscall_name, args) \
	ENTRY(name);                          \
	DO_CALL(syscall_name);                \
	SYSCALL_ERROR_HANDLER

#define ret_PSEUDO
#undef PSEUDO_END
#define PSEUDO_END(name)		END(name)

/*
 * Pseudo-system call; no error massaging
 */
#define PSEUDO_NOERRNO(name, syscall_name, args) \
	ENTRY (name);                                 \
	DO_CALL (syscall_name);

#define	ret_NOERRNO			ret
#undef PSEUDO_END_NOERRNO
#define PSEUDO_END_NOERRNO(name)	END(name)

/*
 * Pseudo-system call; negates return value
 */
#define PSEUDO_ERRVAL(name, syscall_name, args) \
	ENTRY(name)                                  \
	DO_CALL (syscall_name);                      \
	r0 = sub(#0, r0);

#define	ret_ERRVAL			ret
#undef PSEUDO_END_ERRVAL
#define PSEUDO_END_ERRVAL(name)		END(name)

/*
 * syscall error handler; sets errno.
 */
#if NOT_IN_libc
# if RTLD_PRIVATE_ERRNO

/* Use private rtld_errno; manually compute symbol location.
 * Set rtld_errno to -return value, and return -1 to caller.
 */
#  define SYSCALL_ERROR_HANDLER \
1:      { R1 = #-(MAX_KERNEL_ERRNO+1); \
        R2 = PC; \
        R3.H = #HI(1b@GOTOFF); \
        R4.H = #HI(rtld_errno@GOTOFF); \
        } \
        { \
        P3 = cmp.gtu(R0,R1); \
        if (!P3.new) jump:nt 2f; \
        R3.L = #LO(1b@GOTOFF); \
        R4.L = #LO(rtld_errno@GOTOFF); \
        } \
        { \
        R1 = sub(#0, R0); \
        R3 = sub(R2,R3); \
        } \
        { \
        R0 = #-1; \
        R4 = add(R4,R3); \
        } \
        memw(R4) = R1; \
2:
# else /* !RTLD_PRIVATE_ERRNO */

/* Find errno by calling __errno_location to get it.
 * Store -returned value there; return -1 to caller.
 * Mostly used in the nptl code.
 * Note: the call will clobber r31; we must save & restore it.
 */
#  define SYSCALL_ERROR_HANDLER \
        .cfi_def_cfa 30, 8; \
        .cfi_offset 31, -4; \
        .cfi_offset 30, -8; \
        { \
        r2 = sub(#0, r0); \
        r1 = #-(MAX_KERNEL_ERRNO+1); \
        } \
        { \
        p3 = cmp.gtu(r0,r1); \
        if (!p3.new) jump:t 2f; \
        } \
        allocframe(#0); \
        call PLTJMP(__errno_location); \
        { \
        deallocframe; \
        memw(r0) = r2; \
        r0 = #-1; \
        } \
2:
# endif
#else /* !NOT_IN_libc */

/*
 * C implementation to set errno to passed arg; returns -1  in r0.
 * Used mostly by the special-case syscalls
 */
#define SYSCALL_ERROR_HANDLER \
        r1 = #-(MAX_KERNEL_ERRNO+1); \
        { \
        p3 = cmp.gtu(r0,r1); \
        if (!p3.new) jump:t 2f; \
        } \
        r0 = sub(#0, r0); \
        jump PLTJMP(__syscall_error); \
2:
#endif

/* In addition to the above, various hand-written system-call
 * routines need a semi-standardized way of handling returned
 * error values.
 */
#define PSEUDO_RET         \
	SYSCALL_ERROR_HANDLER;  \
	jumpr r31;

#else  //  end ifdef __ASSEMBLER__

#include <errno.h>

#define LOAD_ARGS_0() \
	register ARG_TYPE _r0 __asm__ ("r0");

#define LOAD_ARGS_1(arg1) \
	register ARG_TYPE _r0 __asm__ ("r0") = (ARG_TYPE) arg1;

#define LOAD_ARGS_2(arg1,arg2) \
	LOAD_ARGS_1(arg1) \
	register ARG_TYPE _r1 __asm__ ("r1") = (ARG_TYPE) arg2;

#define LOAD_ARGS_3(arg1,arg2,arg3) \
	LOAD_ARGS_2(arg1,arg2) \
	register ARG_TYPE _r2 __asm__ ("r2") = (ARG_TYPE) arg3;

#define LOAD_ARGS_4(arg1,arg2,arg3,arg4) \
	LOAD_ARGS_3(arg1,arg2,arg3) \
	register ARG_TYPE _r3 __asm__ ("r3") = (ARG_TYPE) arg4;

#define LOAD_ARGS_5(arg1,arg2,arg3,arg4,arg5) \
	LOAD_ARGS_4(arg1,arg2,arg3,arg4) \
	register ARG_TYPE _r4 __asm__ ("r4") = (ARG_TYPE) arg5;

#define LOAD_ARGS_6(arg1,arg2,arg3,arg4,arg5,arg6) \
	LOAD_ARGS_5(arg1,arg2,arg3,arg4,arg5) \
	register ARG_TYPE _r5 __asm__ ("r5") = (ARG_TYPE) arg6;

#define READS_0 \
	"r" (_r6)

#define READS_1 \
	READS_0, "r" (_r0)

#define READS_2 \
	READS_1, "r" (_r1)

#define READS_3 \
	READS_2, "r" (_r2)

#define READS_4 \
	READS_3, "r" (_r3)

#define READS_5 \
	READS_4, "r" (_r4)

#define READS_6 \
	READS_5, "r" (_r5)

#define _syscall0(type,name) \
type name(void) \
{ \
    register ARG_TYPE _r0 __asm__ ("r0"); \
    register long _r6 __asm__ ("r6") = SYS_ify(name); \
    ARG_TYPE _retval; \
    __asm__ __volatile__ ( \
	"trap0(#1);\n"\
	: "=r" (_r0) \
 	: "r" (_r6) \
	: "memory" ); \
    _retval = _r0; \
    if ((int) (unsigned long)_retval >= (unsigned long)-MAX_KERNEL_ERRNO) \
	{ __set_errno(-_retval); _retval = -1; }\
return (type) _retval; \
}

#define _syscall1(type,name,type1,arg1) \
type name(type1 arg1) \
{ \
    register ARG_TYPE _r0 __asm__ ("r0") = (ARG_TYPE) arg1; \
    register long _r6 __asm__ ("r6") = SYS_ify(name); \
    ARG_TYPE _retval; \
    __asm__ __volatile__ ( \
	"trap0(#1);\n"\
	: "=r" (_r0) \
 	: "r" (_r6), \
	  "r" (_r0) \
	: "memory" ); \
    _retval = _r0; \
    if ((int) (unsigned long)_retval >= (unsigned long)-MAX_KERNEL_ERRNO) \
	{ __set_errno(-_retval); _retval = -1; }\
return (type) _retval; \
}

#define _syscall2(type,name,type1,arg1,type2,arg2) \
type name(type1 arg1, type2 arg2) \
{ \
    register ARG_TYPE _r0 __asm__ ("r0") = (ARG_TYPE) arg1; \
    register ARG_TYPE _r1 __asm__ ("r1") = (ARG_TYPE) arg2; \
    register long _r6 __asm__ ("r6") = SYS_ify(name); \
    ARG_TYPE _retval; \
    __asm__ __volatile__ ( \
	"trap0(#1);\n"\
	: "=r" (_r0) \
 	: "r" (_r6), \
	  "r" (_r1), "r" (_r0) \
	: "memory" ); \
    _retval = _r0; \
    if ((int) (unsigned long)_retval >= (unsigned long)-MAX_KERNEL_ERRNO) \
	{ __set_errno(-_retval); _retval = -1; }\
return (type) _retval; \
}

#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1, type2 arg2, type3 arg3) \
{ \
    register ARG_TYPE _r0 __asm__ ("r0") = (ARG_TYPE) arg1; \
    register ARG_TYPE _r1 __asm__ ("r1") = (ARG_TYPE) arg2; \
    register ARG_TYPE _r2 __asm__ ("r2") = (ARG_TYPE) arg3; \
    register long _r6 __asm__ ("r6") = SYS_ify(name); \
    ARG_TYPE _retval; \
    __asm__ __volatile__ ( \
	"trap0(#1);\n"\
	: "=r" (_r0) \
 	: "r" (_r6), \
	  "r" (_r2), "r" (_r1), "r" (_r0) \
	: "memory" ); \
    _retval = _r0; \
    if ((int) (unsigned long)_retval >= (unsigned long)-MAX_KERNEL_ERRNO) \
	{ __set_errno(-_retval); _retval = -1; }\
return (type) _retval; \
}

#define _syscall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4) \
{ \
    register ARG_TYPE _r0 __asm__ ("r0") = (ARG_TYPE) arg1; \
    register ARG_TYPE _r1 __asm__ ("r1") = (ARG_TYPE) arg2; \
    register ARG_TYPE _r2 __asm__ ("r2") = (ARG_TYPE) arg3; \
    register ARG_TYPE _r3 __asm__ ("r3") = (ARG_TYPE) arg4; \
    register long _r6 __asm__ ("r6") = SYS_ify(name); \
    ARG_TYPE _retval; \
    __asm__ __volatile__ ( \
	"trap0(#1);\n"\
	: "=r" (_r0) \
 	: "r" (_r6), "r" (_r3), \
	  "r" (_r2), "r" (_r1), "r" (_r0) \
	: "memory" ); \
    _retval = _r0; \
    if ((int) (unsigned long)_retval >= (unsigned long)-MAX_KERNEL_ERRNO) \
	{ __set_errno(-_retval); _retval = -1; }\
return (type) _retval; \
}

#define _syscall5(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5) \
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5) \
{ \
    register ARG_TYPE _r0 __asm__ ("r0") = (ARG_TYPE) arg1; \
    register ARG_TYPE _r1 __asm__ ("r1") = (ARG_TYPE) arg2; \
    register ARG_TYPE _r2 __asm__ ("r2") = (ARG_TYPE) arg3; \
    register ARG_TYPE _r3 __asm__ ("r3") = (ARG_TYPE) arg4; \
    register ARG_TYPE _r4 __asm__ ("r4") = (ARG_TYPE) arg5; \
    register long _r6 __asm__ ("r6") = SYS_ify(name); \
    ARG_TYPE _retval; \
    __asm__ __volatile__ ( \
	"trap0(#1);\n"\
	: "=r" (_r0) \
 	: "r" (_r6), "r" (_r4), "r" (_r3), \
	  "r" (_r2), "r" (_r1), "r" (_r0) \
	: "memory" ); \
    _retval = _r0; \
    if ((int) (unsigned long)_retval >= (unsigned long)-MAX_KERNEL_ERRNO) \
	{ __set_errno(-_retval); _retval = -1; }\
	return (type) _retval; \
}

#define _syscall6(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5,type6,arg6) \
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6) \
{ \
    register ARG_TYPE _r0 __asm__ ("r0") = (ARG_TYPE) arg1; \
    register ARG_TYPE _r1 __asm__ ("r1") = (ARG_TYPE) arg2; \
    register ARG_TYPE _r2 __asm__ ("r2") = (ARG_TYPE) arg3; \
    register ARG_TYPE _r3 __asm__ ("r3") = (ARG_TYPE) arg4; \
    register ARG_TYPE _r4 __asm__ ("r4") = (ARG_TYPE) arg5; \
    register ARG_TYPE _r5 __asm__ ("r5") = (ARG_TYPE) arg6; \
    register long _r6 __asm__ ("r6") = SYS_ify(name); \
    ARG_TYPE _retval; \
    __asm__ __volatile__ ( \
	"trap0(#1);\n"\
	: "=r" (_r0) \
 	: "r" (_r6), "r" (_r5), "r" (_r4), "r" (_r3), \
	  "r" (_r2), "r" (_r1), "r" (_r0) \
	: "memory" ); \
    _retval = _r0; \
    if ((int) (unsigned long)_retval >= (unsigned long)-MAX_KERNEL_ERRNO) \
	{ __set_errno(-_retval); _retval = -1; }\
	return (type) _retval; \
}

#undef INTERNAL_SYSCALL_DECL
#define INTERNAL_SYSCALL_DECL(err) do {} while (0);

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err) \
	((int) (unsigned long) (val) >= (unsigned long)-MAX_KERNEL_ERRNO)

#define INTERNAL_SYSCALL_ERRNO(val, err)	(-(val))

/* Define a macro which expands inline into the wrapper code for a system
   call. This use is for internal calls that do not need to handle errors
   normally. It will never touch errno.  */

#define INTERNAL_SYSCALL_NCS(no, err, nr, args...) \
({ \
	LOAD_ARGS_##nr(args)\
	register long _r6 __asm__ ("r6") = (long) (no); \
	ARG_TYPE _retval; \
	__asm__ __volatile__ ( \
	"trap0(#1);\n"\
		: "=r" (_r0) \
		: READS_##nr \
		: "memory" ); \
	_retval = _r0; \
	(int) _retval;\
})

#undef INTERNAL_SYSCALL
#define INTERNAL_SYSCALL(name, err, nr, args...) \
  INTERNAL_SYSCALL_NCS (SYS_ify(name), err, nr, ##args)


/*  borrowed from MIPS  */
#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)                             \
  ({ INTERNAL_SYSCALL_DECL(err);                                      \
     long result_var = INTERNAL_SYSCALL (name, err, nr, args);        \
     if ( INTERNAL_SYSCALL_ERROR_P (result_var, err) )                \
       {                                                              \
         __set_errno (INTERNAL_SYSCALL_ERRNO (result_var, err));      \
         result_var = -1L;                                            \
       }                                                              \
     result_var; })

#endif  // ! ifdef __ASSEMBLY__


/* Pointer mangling support.  */
/* Currently disabled; some implementation work remains in setjmp. */
#ifdef PTR_MANGLE_SOME_OTHER_DAY
#if defined NOT_IN_libc && defined IS_IN_rtld
/* We cannot perform ptr mangling in ld.so because setjmp() is used
   before before the descriptor is initialized.  */
#else
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(reg, tmpreg) \
	tmpreg = memw(r25 + #POINTER_GUARD); \
	reg = xor(reg,tmpreg);
#  define PTR_MANGLE2(reg, tmpreg) \
	reg = xor(reg,tmpreg);
#  define PTR_MANGLE3(destreg, reg, tmpreg) \
	tmpreg = memw(r25 + #POINTER_GUARD); \
	destreg = xor(reg,tmpreg);
#  define PTR_DEMANGLE(reg, tmpreg) PTR_MANGLE (reg, tmpreg)
#  define PTR_DEMANGLE2(reg, tmpreg) PTR_MANGLE2 (reg, tmpreg)
#  define PTR_DEMANGLE3(destreg, reg, tmpreg) PTR_MANGLE3 (destreg, reg, tmpreg)
# else
#  define PTR_MANGLE(var) \
  (var) = (__typeof (var)) ((uintptr_t) (var) ^ THREAD_GET_POINTER_GUARD ())
#  define PTR_DEMANGLE(var)	PTR_MANGLE (var)
# endif
#endif
#else // PTR_MANGLE_SOME_OTHER_DAY
# define PTR_MANGLE(var)
# define PTR_DEMANGLE(var)
#endif // PTR_MANGLE_SOME_OTHER_DAY
#endif  // end ifndef __BITS_SYSCALLS_H

