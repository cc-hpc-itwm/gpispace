#include <sdpa/modules/Macros.hpp>
#include <iostream>
#include <string>
#include <cstdlib> // malloc, free
#include <fvm-pc/pc.hpp>
#include <Pv4dLogger.h>
#include <stdexcept>

using namespace sdpa::modules;

static void RunTest() throw (std::exception)
{
  std::cout << "running RunTest example" << std::endl;
  pv4d_printf("this is a pv4d-log-test\n");
}

SDPA_MOD_INIT_START(pc-test)
{
  // run test cases
  RunTest();
}
SDPA_MOD_INIT_END(pc-test)
