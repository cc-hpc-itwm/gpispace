#include <sdpa/modules/Macros.hpp>
#include <iostream>
#include <string>
#include <cstdlib> // malloc, free
#include <fvm-pc/pc.hpp>

using namespace sdpa::modules;

static void RunTest() throw (std::exception)
{
  std::cout << "running RunTest example" << std::endl;
  fvmAllocHandle_t src = fvmGlobalAlloc((256*1024*1024));
  fvmGlobalFree(src);
  std::cout << "shmem-size: " << fvmGetShmemSize() << std::endl;
}

SDPA_MOD_INIT_START(pc-test)
{
  // run test cases
  RunTest();
}
SDPA_MOD_INIT_END(pc-test)
