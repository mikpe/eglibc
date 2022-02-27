#include <stdio.h>
#include <string.h>

#define TEST_FUNCTION do_test ()
static int
do_test (void)
{
  const char haystack[] = "AOKB";
  const char needle[] = "OK";
  const char *sub = strcasestr (haystack, needle);

  if (sub == NULL)
    {
      fprintf (stderr, "BUG: didn't find \"%s\" in \"%s\"\n", needle, haystack);
      return 1;
    }

  return 0;
}

#include "../test-skeleton.c"
