
#include "hello_util.h"
#include <stdio.h>
#include <unistd.h>

void hello_util (void)
{
  printf ("*** Holla");

  fflush (stdout);

  sleep (1);

  printf (" util\n");
}
