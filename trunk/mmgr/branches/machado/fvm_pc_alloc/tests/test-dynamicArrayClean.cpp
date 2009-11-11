#include <string>
#include <assert.h>

#include <Module.hpp>

#include <pc.h>
#include <Pv4dLogger.h>

using namespace sdpa::modules;
using namespace std;

void test_dynamicArrayClean(Module::data_t &params)
{

  printf("*******************************************************************************\n");
  printf("---------------------- Test Dynamic Array Clean -------------------------------\n");
  printf("*******************************************************************************\n");

  fvmAllocHandle_t scratch;
  ssize_t S = 1024;
  fvmCommHandle_t commH;
  fvmShmemOffset_t L = 0;
  int r = fvmGetRank();

  fvmAllocHandle_t handle = params["handle"].token().data_as<fvmAllocHandle_t>();
  printf("handle received is %lu\n", handle);

  fvmGlobalFree(handle);

}


// The init function will call all tests implemented
extern "C" {
  void sdpa_mod_init(Module *mod) {
    SDPA_REGISTER_FUN(mod, test_dynamicArrayClean);

    //actually call the functions
    //    sdpa::modules::Module::data_t params;
    //    test_dynamicArrayUsage(params);

  }

}
