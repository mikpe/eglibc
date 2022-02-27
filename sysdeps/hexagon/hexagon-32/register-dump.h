/* Dump registers.
   Copyright (C) 1998, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <sys/uio.h>
#include <stdio-common/_itoa.h>

/* The dumpform is the format of the printed dump; the numbering here
 * must match that of struct user_regs_struct, exactly. */
static const char dumpform[] = "\
Register dump:\n\
r0 =0000000% r1 =0000001% r2 =0000002% r3 =0000003% preds=0000027%\n\
r4 =0000004% r5 =0000005% r6 =0000006% r7 =0000007%   sa0=0000020% sa1=0000022%\n\
r8 =0000008% r9 =0000009% r10=000000a% r11=000000b%   lc0=0000021% lc1=0000023%\n\
r12=000000c% r13=000000d% r14=000000e% r15=000000f%   m0 =0000024% m1 =0000025%\n\
r16=0000010% r17=0000011% r18=0000012% r19=0000013%   usr=0000026%\n\
r20=0000014% r21=0000015% r22=0000016% r23=0000017%   gp =0000028% ugp=0000029%\n\
r24=0000018% r25=0000019% r26=000001a% r27=000001b% cause=000002b% pc=000002a%\n\
r28=000001c% sp =000001d% fp =000001e% r31=000001f% badva=000002c%\n\
";

#define xtoi(x) (x >= 'a' ? x + 10 - 'a' : x - '0')

static void
register_dump (int fd, const SIGCONTEXT ctx)
{
  char buffer[sizeof(dumpform)];
  char *bufferpos;
  unsigned regno;

  /* Normally, this is struct user_regs_struct */
  unsigned *regs = (unsigned *)(&ctx->sc_regs);

  memcpy(buffer, dumpform, sizeof(dumpform));

  /* Generate the output.  */
  while ((bufferpos = memchr (buffer, '%', sizeof(dumpform))))
    {
      regno = xtoi (bufferpos[-1]) | xtoi (bufferpos[-2]) << 4;
      memset (bufferpos-2, '0', 3);
      _itoa_word (regs[regno], bufferpos+1, 16, 0);
    }

  /* Write the output.  */
  write (fd, buffer, sizeof(buffer) - 1);
}

#define REGISTER_DUMP \
  register_dump (fd, ctx)
