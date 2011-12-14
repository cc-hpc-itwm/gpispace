#include <sdpa/modules/Macros.hpp>
#include <iostream>
#include <string>
#include <cstdlib> // malloc, free
#include <fvm-pc/pc.hpp>
#include <stdexcept>

using namespace sdpa::modules;

static void RunTest() throw (std::exception)
{
  std::cout << "running RunTest example" << std::endl;
  fvmAllocHandle_t cfg = fvmGlobalAlloc((256*1024*1024));
  std::cout << "got = " << cfg << std::endl;
  if (!cfg)
  {
    throw std::runtime_error("allocation failed");
  }
}

SDPA_MOD_INIT_START(pc-test)
{
  // run test cases
  RunTest();
}
SDPA_MOD_INIT_END(pc-test)
