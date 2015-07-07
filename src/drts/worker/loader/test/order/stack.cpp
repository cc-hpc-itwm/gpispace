#include <drts/worker/loader/test/order/stack.hpp>

std::stack<std::string>& start()
{
  static std::stack<std::string> s;

  return s;
}
