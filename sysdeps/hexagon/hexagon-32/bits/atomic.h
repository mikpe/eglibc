
/* Atomic operations.  Hexagon-32 version.
   Copyright (C) 2003, 2004, 2007, 2010 Free Software Foundation, Inc.
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


/* Hexagon does not have a 64-bit (doubleword) forms load locked
   and store conditional (memw_locked) instructions. So stub out
   the 64-bit forms.  */

#define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  (abort (), (__typeof (*mem)) 0)

#define __arch_compare_and_exchange_val_64_rel(mem, newval, oldval) \
  (abort (), (__typeof (*mem)) 0)

#define __arch_atomic_exchange_64_acq(mem, value) \
    ({ abort (); (*mem) = (value); })

#define __arch_atomic_exchange_64_rel(mem, value) \
    ({ abort (); (*mem) = (value); })

#define __arch_atomic_exchange_and_add_64(mem, value) \
    ({ abort (); (*mem) = (value); })

#define __arch_atomic_increment_val_64(mem) \
    ({ abort (); (*mem)++; })

#define __arch_atomic_decrement_val_64(mem) \
    ({ abort (); (*mem)--; })

#define __arch_atomic_decrement_if_positive_64(mem) \
    ({ abort (); (*mem)--; })

/*
 * Include the rest of the atomic ops macros which are common to both
 * hexagon-32 and hexagon-64.
 */
#include_next <bits/atomic.h>
