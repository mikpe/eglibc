/* Optimized memmove implementation for Hexagon V3 core.
 * Copyright (c) 2009, 2010, 2011 QUALCOMM INCORPORATED.
 * All Rights Reserved.
 */

#include <string.h>

void *_hexagon_reverse_memcpy(void *dest, const void *src, size_t n);
void *_hexagon_forward_memcpy(void *dest, const void *src, size_t n);

#ifndef  a1
#define  a1 dest  /* First arg is DEST.  */
#define  a1const
#define  a2 src   /* Second arg is SRC.  */
#define  a2const  const
#undef memmove
#endif
#if   !defined(RETURN) || !defined(rettype)
#define  RETURN(s)   return (s)  /* Return DEST.  */
#define  rettype     void *
#endif


/**
 * memmove function
 */
rettype
memmove(a1const void * a1, a2const void * a2, size_t length)
{
  void * a;
  unsigned long int isrc = (unsigned long int) src;
  unsigned long int idst = (unsigned long int) dest;

  /* Perform normal memcpy if src is higher in memory than dest or
   * they are at least the length of the transfer away from each other.
   */
  if (isrc - idst > length ||
      idst - isrc > length) {
    a = _hexagon_forward_memcpy(dest, src, length);
  } else {
    a = _hexagon_reverse_memcpy(dest, src, length);
  }
  RETURN(a);
}

#ifndef memmove
libc_hidden_builtin_def (memmove)
#endif

