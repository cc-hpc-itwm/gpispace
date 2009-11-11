#include <sdpa/modules/Module.hpp>
#include <iostream>
#include <string>
#include <cstdlib> // malloc, free

#include "mod-a.hpp"

int fun_a(int p)
{
  std::cout << "fun_a(" << p << ")" << std::endl;
  return 0;
}

using namespace sdpa::modules;

static void RunTest() throw (std::exception)
{
  std::cout << "running RunTest example" << std::endl;
  fun_a(0);
}

SDPA_MOD_INIT_START(mod-a)
{
  // run test cases
  RunTest();
}
SDPA_MOD_INIT_END(mod-a)
