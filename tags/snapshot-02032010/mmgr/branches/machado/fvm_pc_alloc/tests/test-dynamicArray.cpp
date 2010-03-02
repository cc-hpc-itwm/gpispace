#include <string>
#include <assert.h>

#include <Module.hpp>

#include <pc.h>
#include <Pv4dLogger.h>

using namespace sdpa::modules;
using namespace std;


void init(void *a, ssize_t S)
{
  memset(a, 1, S); 
}


//Test dynamic array example

void test_dynamicArrayInit(Module::data_t &params)
{
  printf("*******************************************************************************\n");
  printf("---------------------- Test Dynamic Array Init --------------------------------\n");
  printf("*******************************************************************************\n");

 
  ssize_t S = 1024;
  fvmCommHandle_t commH;
  fvmShmemOffset_t L = 0;

  fvmAllocHandle_t handle;
  fvmAllocHandle_t scratch;
  handle = fvmGlobalAlloc (S);             // S bytes on each node
  scratch = fvmLocalAlloc( S);
  int * shmemPtr = (int *) fvmGetShmemPtr(); // start of local shmem segment
  init (shmemPtr, S);                        // fill in the values

  fvmPutGlobalData(handle, 0, S, L, scratch); 

  printf("Handle to use is %lu\n", handle);
  params["out"].token().data(handle);


}
  

// The init function will call all tests implemented
extern "C" {
  void sdpa_mod_init(Module *mod) {
    SDPA_REGISTER_FUN(mod, test_dynamicArrayInit);

    //actually call the functions
    //    sdpa::modules::Module::data_t params;
    //    test_dynamicArrayInit(params);

  }
}
