#include "guard.h"
#include "req-guard.h" // this should simulate dlopen behaviour!?

#include <stdio.h>

int main ()
{
  printf ("guard value := %d\n", request_guard ());
  return 0;
}
