#include <we/loader/test/order/stack.hpp>

#include <we/loader/macros.hpp>

WE_MOD_INITIALIZE_START (b);
{
  start().push ("b");
}
WE_MOD_INITIALIZE_END (b);
