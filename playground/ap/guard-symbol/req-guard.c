#include "guard.h"
#include "req-guard.h"

// create a reference to the guard
static int s_use_guard ()
{
  return MAKE_SYMBOL(SYMBOL_VALUE);
}

// use the static function
int request_guard ()
{
  return s_use_guard ();
}
