#include <drts/worker/loader/test/order/stack.hpp>

#include <drts/worker/loader/wrapper.hpp>

WE_MOD_INITIALIZE_START (a);
{
  start().push ("a");
}
WE_MOD_INITIALIZE_END (a);
