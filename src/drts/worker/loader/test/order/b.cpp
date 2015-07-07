#include <drts/worker/loader/test/order/stack.hpp>

#include <drts/worker/loader/wrapper.hpp>

WE_MOD_INITIALIZE_START (b);
{
  start().push ("b");
}
WE_MOD_INITIALIZE_END (b);
