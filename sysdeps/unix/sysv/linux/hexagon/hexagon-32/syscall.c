#include <features.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdarg.h>

long syscall(long sysnum, ...)
{
  va_list args;
  long arg1, arg2, arg3, arg4, arg5, arg6;

  va_start(args, sysnum);
  arg1 = va_arg(args, long);
  arg2 = va_arg(args, long);
  arg3 = va_arg(args, long);
  arg4 = va_arg(args, long);
  arg5 = va_arg(args, long);
  arg6 = va_arg(args, long);
  va_end(args);

  register ARG_TYPE _r0 __asm__ ("r0") = arg1;
  register ARG_TYPE _r1 __asm__ ("r1") = arg2;
  register ARG_TYPE _r2 __asm__ ("r2") = arg3;
  register ARG_TYPE _r3 __asm__ ("r3") = arg4;
  register ARG_TYPE _r4 __asm__ ("r4") = arg5;
  register ARG_TYPE _r5 __asm__ ("r5") = arg6;
  register long _r6 __asm__ ("r6") = sysnum;
  long _retval;
  __asm__ __volatile__ (
			"trap0(#1);\n"
			: "=r" (_r0)
			: "r" (_r6), "r" (_r5), "r" (_r4), "r" (_r3),
			  "r" (_r2), "r" (_r1), "r" (_r0)
			: "memory" );
  _retval = _r0;
  if ((int) (unsigned long)_retval >= (unsigned long)-MAX_KERNEL_ERRNO)
    { __set_errno(-_retval); _retval = -1; }
  return _retval;
}
