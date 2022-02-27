
/* Atomic operations.  Hexagon version.
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

/*
 * Never include sysdeps/hexagon/bits/atomic.h directly.
 * Alway use include/atomic.h which will include either
 * sysdeps/hexagon/hexagon-32/bits/atomic.h
 * or
 * sysdeps/hexagon/hexagon-64/bits/atomic.h
 * as appropriate and which in turn include this file.
 */

#include <stdint.h>

typedef int32_t atomic32_t;
typedef uint32_t uatomic32_t;
typedef int_fast32_t atomic_fast32_t;
typedef uint_fast32_t uatomic_fast32_t;

typedef int64_t atomic64_t;
typedef uint64_t uatomic64_t;
typedef int_fast64_t atomic_fast64_t;
typedef uint_fast64_t uatomic_fast64_t;

typedef intptr_t atomicptr_t;
typedef uintptr_t uatomicptr_t;
typedef intmax_t atomic_max_t;
typedef uintmax_t uatomic_max_t;

/*
 * Hexagon does not have byte and halfword forms of load and reserve and
 * store conditional. So for hexagon we stub out the 8- and 16-bit forms.
 */
#define __arch_compare_and_exchange_bool_8_acq(mem, newval, oldval) \
  (abort (), 0)

#define __arch_compare_and_exchange_bool_16_acq(mem, newval, oldval) \
  (abort (), 0)

#define __arch_compare_and_exchange_bool_8_rel(mem, newval, oldval) \
  (abort (), 0)

#define __arch_compare_and_exchange_bool_16_rel(mem, newval, oldval) \
  (abort (), 0)

#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval) \
  ({                                              \
    __typeof (*(mem)) __tmp;                      \
    asm volatile(                                 \
      "1: %0 = memw_locked(%2);\n"                \
      "  {\n"                                     \
      "    P3 = cmp.eq(%0,%3);\n"                 \
      "    if (!P3.new) jump:nt 2f; \n"           \
      "  }\n"                                     \
      "  memw_locked(%2,P3) = %4;\n"              \
      "  if (!P3) jump 1b;\n"                     \
      "2:\n"                                      \
      : "=&r" (__tmp), "+m"(*mem)                 \
      : "r" (mem), "r" (oldval), "r" (newval)     \
      : "p3"                                      \
    );                                            \
    __tmp;                                        \
  })

#define __arch_compare_and_exchange_val_32_rel(mem, newval, oldval) \
  __arch_compare_and_exchange_val_32_acq (mem, newval, oldval)

#define __arch_atomic_exchange_32_acq(mem, value) \
  ({                                              \
    __typeof (*mem) __val;                        \
    asm volatile (                                \
      "1:  %0 = memw_locked(%2);\n"               \
      "  memw_locked(%2,P3) = %3;\n"              \
      "  if (!P3) jump 1b;\n"                     \
      : "=&r" (__val), "+m" (*mem)                \
      : "r" (mem), "r" (value)                    \
      : "p3"                                      \
    );                                            \
    __val;                                        \
  })

#define __arch_atomic_exchange_32_rel(mem, value) \
  __arch_atomic_exchange_32_acq(mem, value)

#define __arch_atomic_exchange_and_add_32(mem, value) \
  ({                                              \
    __typeof (*mem) __val;                        \
    __typeof (*mem) __old_val;                    \
    asm volatile (                                \
      "1:  %2 = memw_locked(%3);\n"               \
      "  %0 = add(%2,%4);\n"                      \
      "  memw_locked(%3,P3) = %0;\n"              \
      "  if (!P3) jump 1b;\n"                     \
      : "=&r" (__val), "+m" (*mem), "=&r" (__old_val) \
      : "r" (mem), "r" (value)                    \
      : "p3"                                      \
    );                                            \
    __old_val;                                    \
  })

#define __arch_atomic_increment_val_32(mem)       \
  ({                                              \
    __typeof (*mem) __val;                        \
    asm volatile (                                \
      "1:  %0 = memw_locked(%2);\n"               \
      "  %0 = add(%0,#1);\n"                      \
      "  memw_locked(%2,P3) = %0;\n"              \
      "  if (!P3) jump 1b;\n"                     \
      : "=&r" (__val), "+m"(*mem)                 \
      : "r" (mem)                                 \
      : "p3"                                      \
    );                                            \
    __val;                                        \
  })

#define __arch_atomic_decrement_val_32(mem)       \
  ({                                              \
    __typeof (*mem) __val;                        \
    asm volatile (                                \
      "1:  %0 = memw_locked(%2);\n"               \
      "  %0 = add(%0,#-1);\n"                     \
      "  memw_locked(%2,P3) = %0;\n"              \
      "  if (!P3) jump 1b;\n"                     \
      : "=&r" (__val), "+m"(*mem)                 \
      : "r"(mem)                                  \
      : "p3"                                      \
    );                                            \
    __val;                                        \
  })

#define __arch_atomic_decrement_if_positive_32(mem) \
  ({                                                \
    __typeof (*mem) __val;                          \
    __typeof (*mem) __old_val;                      \
    asm volatile (                                  \
      "1:  %0 = memw_locked(%3);\n"                 \
      "  P3 = cmp.gt(%0,#0);\n"                     \
      "  if (!P3) jump 2f;\n"                       \
      "  %1 = add(%0,#-1);\n"                       \
      "  memw_locked(%3,P3) = %1;\n"                \
      "  if (!P3) jump 1b;\n"                       \
      "2:\n"                                        \
      : "=&r" (__old_val), "=&r" (__val), "+m" (*mem)\
      : "r" (mem)                                   \
      : "p3"                                        \
    );                                              \
    __old_val;                                      \
  })


#define atomic_compare_and_exchange_val_acq(mem, newval, oldval)              \
  ({                                                                          \
    __typeof (*(mem)) __result;                                               \
    if (sizeof (*mem) == 4)                                                   \
      __result = __arch_compare_and_exchange_val_32_acq(mem, newval, oldval); \
    else if (sizeof (*mem) == 8)                                              \
      __result = __arch_compare_and_exchange_val_64_acq(mem, newval, oldval); \
    else                                                                      \
       abort ();                                                              \
    __result;                                                                 \
  })

#define atomic_compare_and_exchange_val_rel(mem, newval, oldval)              \
  ({                                                                          \
    __typeof (*(mem)) __result;                                               \
    if (sizeof (*mem) == 4)                                                   \
      __result = __arch_compare_and_exchange_val_32_rel(mem, newval, oldval); \
    else if (sizeof (*mem) == 8)                                              \
      __result = __arch_compare_and_exchange_val_64_rel(mem, newval, oldval); \
    else                                                                      \
       abort ();                                                              \
    __result;                                                                 \
  })

#define atomic_exchange_acq(mem, value)                                       \
  ({                                                                          \
    __typeof (*(mem)) __result;                                               \
    if (sizeof (*mem) == 4)                                                   \
      __result = __arch_atomic_exchange_32_acq (mem, value);                  \
    else if (sizeof (*mem) == 8)                                              \
      __result = __arch_atomic_exchange_64_acq (mem, value);                  \
    else                                                                      \
       abort ();                                                              \
    __result;                                                                 \
  })

#define atomic_exchange_rel(mem, value)                                       \
  ({                                                                          \
    __typeof (*(mem)) __result;                                               \
    if (sizeof (*mem) == 4)                                                   \
      __result = __arch_atomic_exchange_32_rel (mem, value);                  \
    else if (sizeof (*mem) == 8)                                              \
      __result = __arch_atomic_exchange_64_rel (mem, value);                  \
    else                                                                      \
       abort ();                                                              \
    __result;                                                                 \
  })

#define atomic_exchange_and_add(mem, value)                                   \
  ({                                                                          \
    __typeof (*(mem)) __result;                                               \
    if (sizeof (*mem) == 4)                                                   \
      __result = __arch_atomic_exchange_and_add_32 (mem, value);              \
    else if (sizeof (*mem) == 8)                                              \
      __result = __arch_atomic_exchange_and_add_64 (mem, value);              \
    else                                                                      \
       abort ();                                                              \
    __result;                                                                 \
  })

#define atomic_increment_val(mem) \
  ({                                                                          \
    __typeof (*(mem)) __result;                                               \
    if (sizeof (*(mem)) == 4)                                                 \
      __result = __arch_atomic_increment_val_32 (mem);                        \
    else if (sizeof (*(mem)) == 8)                                            \
      __result = __arch_atomic_increment_val_64 (mem);                        \
    else                                                                      \
       abort ();                                                              \
    __result;                                                                 \
  })

#define atomic_increment(mem) ({ atomic_increment_val (mem); (void) 0; })

#define atomic_decrement_val(mem) \
  ({                                                                          \
    __typeof (*(mem)) __result;                                               \
    if (sizeof (*(mem)) == 4)                                                 \
      __result = __arch_atomic_decrement_val_32 (mem);                        \
    else if (sizeof (*(mem)) == 8)                                            \
      __result = __arch_atomic_decrement_val_64 (mem);                        \
    else                                                                      \
       abort ();                                                              \
    __result;                                                                 \
  })

#define atomic_decrement(mem) ({ atomic_decrement_val (mem); (void) 0; })


/* Decrement *MEM if it is > 0, and return the old value.  */
#define atomic_decrement_if_positive(mem) \
  ({ __typeof (*(mem)) __result;                                              \
    if (sizeof (*mem) == 4)                                                   \
      __result = __arch_atomic_decrement_if_positive_32 (mem);                \
    else if (sizeof (*mem) == 8)                                              \
      __result = __arch_atomic_decrement_if_positive_64 (mem);                \
    else                                                                      \
       abort ();                                                              \
    __result;                                                                 \
  })
