#include "get_cpucount.h"

#include <unistd.h>

int fhg_get_cpucount ()
{
  return sysconf (_SC_NPROCESSORS_ONLN);
}
