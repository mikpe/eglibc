#include_next <eglibc/dynconf.h>

#if defined(__OPTION_EGLIBC_DYNAMIC_CONF) && !defined(NOT_IN_libc)
/* Declare the internal interfaces.  */
extern __typeof (eglibc_getconf) __eglibc_getconf;
extern __typeof (eglibc_setconf) __eglibc_setconf;
libc_hidden_proto (__eglibc_getconf);
libc_hidden_proto (__eglibc_setconf);

# undef EGLIBC_BUFSIZ
# undef EGLIBC_THREAD_STACK_MIN
# define EGLIBC_BUFSIZ (__eglibc_getconf (EGLIBC_CONF_BUFSIZ))
# define EGLIBC_THREAD_STACK_MIN \
  (__eglibc_getconf (EGLIBC_CONF_THREAD_STACK_MIN))
#endif

