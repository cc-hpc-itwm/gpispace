#include <sdpa/modules/Macros.hpp>
#include <iostream>
#include <string>
#include <cstdlib> // malloc, free

#include "a-mod.hpp"
#include "b-mod.hpp"

int fun_b(int p)
{
  std::cout << "fun_b(" << p << ")" << std::endl;
  return p + fun_a(p);
}

using namespace sdpa::modules;

static void RunTest() throw (std::exception)
{
  std::cout << "running RunTest example" << std::endl;
  fun_b(42);
}

SDPA_MOD_INIT_START(b-mod)
{
  // run test cases
  RunTest();
}
SDPA_MOD_INIT_END(b-mod)
