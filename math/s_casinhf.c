/* Return arc hyperbole sine for float value.
   Copyright (C) 1997-2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <complex.h>
#include <math.h>
#include <math_private.h>
#include <float.h>

__complex__ float
__casinhf (__complex__ float x)
{
  __complex__ float res;
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  if (rcls <= FP_INFINITE || icls <= FP_INFINITE)
    {
      if (icls == FP_INFINITE)
	{
	  __real__ res = __copysignf (HUGE_VALF, __real__ x);

	  if (rcls == FP_NAN)
	    __imag__ res = __nanf ("");
	  else
	    __imag__ res = __copysignf (rcls >= FP_ZERO ? M_PI_2 : M_PI_4,
					__imag__ x);
	}
      else if (rcls <= FP_INFINITE)
	{
	  __real__ res = __real__ x;
	  if ((rcls == FP_INFINITE && icls >= FP_ZERO)
	      || (rcls == FP_NAN && icls == FP_ZERO))
	    __imag__ res = __copysignf (0.0, __imag__ x);
	  else
	    __imag__ res = __nanf ("");
	}
      else
	{
	  __real__ res = __nanf ("");
	  __imag__ res = __nanf ("");
	}
    }
  else if (rcls == FP_ZERO && icls == FP_ZERO)
    {
      res = x;
    }
  else
    {
      float rx, ix;
      __complex__ float y;

      /* Avoid cancellation by reducing to the first quadrant.  */
      rx = fabsf (__real__ x);
      ix = fabsf (__imag__ x);

      if (rx >= 1.0f / FLT_EPSILON || ix >= 1.0f / FLT_EPSILON)
	{
	  /* For large x in the first quadrant, x + csqrt (1 + x * x)
	     is sufficiently close to 2 * x to make no significant
	     difference to the result; avoid possible overflow from
	     the squaring and addition.  */
	  __real__ y = rx;
	  __imag__ y = ix;
	  res = __clogf (y);
	  __real__ res += (float) M_LN2;
	}
      else
	{
	  __real__ y = (rx - ix) * (rx + ix) + 1.0;
	  __imag__ y = 2.0 * rx * ix;

	  y = __csqrtf (y);

	  __real__ y += rx;
	  __imag__ y += ix;

	  res = __clogf (y);
	}

      /* Give results the correct sign for the original argument.  */
      __real__ res = __copysignf (__real__ res, __real__ x);
      __imag__ res = __copysignf (__imag__ res, __imag__ x);
    }

  return res;
}
#ifndef __casinhf
weak_alias (__casinhf, casinhf)
#endif
