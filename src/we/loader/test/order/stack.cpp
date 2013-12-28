#include "stack.hpp"

std::stack<std::string>& start()
{
  static std::stack<std::string> s;

  return s;
}
std::stack<std::string>& stop()
{
  static std::stack<std::string> s;

  return s;
}
