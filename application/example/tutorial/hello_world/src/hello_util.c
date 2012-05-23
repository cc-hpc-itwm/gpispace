
#include "hello_util.h"
#include <stdio.h>
#include <unistd.h>

void hello_util (void)
{
  printf ("*** (LIB) Hola");

  fflush (stdout);

  sleep (1);

  printf (" mundo *** (LIB)\n");
}
