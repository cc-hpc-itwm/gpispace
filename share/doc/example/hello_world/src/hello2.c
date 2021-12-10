
#include "hello2.h"
#include <stdio.h>
#include <unistd.h>

void hello_world (void)
{
  printf ("*** (C) Hallo");

  fflush (stdout);

  sleep (1);

  printf (" Welt *** (C)\n");
}
