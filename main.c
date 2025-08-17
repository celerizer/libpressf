#if PF_TEST_BUILD

#include "src/config.h"

#include <stdio.h>

int main(void)
{
  unsigned int x = 1;
  int is_little_endian = (*(unsigned char *)&x == 1);

#if PF_BIG_ENDIAN
  if (is_little_endian)
  {
    printf("Config expects BIG endian, but machine is LITTLE endian!\n");
    return 1;
  }
#else
  if (!is_little_endian)
  {
    printf("Config expects LITTLE endian, but machine is BIG endian!\n");
    return 1;
  }
#endif

  return 0;
}

#endif
