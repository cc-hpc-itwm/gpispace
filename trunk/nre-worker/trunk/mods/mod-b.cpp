#include <sdpa/modules/Macros.hpp>
#include <iostream>
#include <string>
#include <cstdlib> // malloc, free

#include "mod-a.hpp"
#include "mod-b.hpp"

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

SDPA_MOD_INIT_START(mod-b)
{
  // run test cases
  RunTest();
}
SDPA_MOD_INIT_END(mod-b)
