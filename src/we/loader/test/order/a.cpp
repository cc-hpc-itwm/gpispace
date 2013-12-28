#include "stack.hpp"

#include <we/loader/macros.hpp>

WE_MOD_INITIALIZE_START (a);
{
  start().push ("a");
}
WE_MOD_INITIALIZE_END (a);

WE_MOD_FINALIZE_START (a);
{
  stop().push ("a");
}
WE_MOD_FINALIZE_END (a);
