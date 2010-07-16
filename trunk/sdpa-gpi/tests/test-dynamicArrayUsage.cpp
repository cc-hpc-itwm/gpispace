#include <string>
#include <assert.h>

#include <Module.hpp>

#include <pc.h>
#include <Pv4dLogger.h>

using namespace sdpa::modules;
using namespace std;

void test_dynamicArrayUsage(Module::data_t &params)
{

  printf("*******************************************************************************\n");
  printf("---------------------- Test Dynamic Array Usage -------------------------------\n");
  printf("*******************************************************************************\n");

  fvmAllocHandle_t scratch;
  ssize_t S = 1024;
  fvmCommHandle_t commH;
  fvmShmemOffset_t L = 0;
  int r = fvmGetRank();

  fvmAllocHandle_t handle = params["handle"].token().data_as<fvmAllocHandle_t>();
  printf("handle received is %lu\n", handle);


  scratch = fvmLocalAlloc (S);

  commH = fvmGetGlobalData (handle, 0, S, L, scratch);

  waitComm(commH);

  fvmLocalFree(scratch);
  // the array is copied to the local shmem segment now

  char * shmemPtr = (char *) fvmGetShmemPtr();
  char * globalArray = shmemPtr + L;    // this is the begin of the array in the local shmem segment

  for(int i=0; i < S; i++)
    {
      if(globalArray[i] != 1)
	{
	  printf("Value is wrong %c\n", globalArray[i]);
	  break;
	}
    }

}


// The init function will call all tests implemented
extern "C" {
  void sdpa_mod_init(Module *mod) {
    SDPA_REGISTER_FUN(mod, test_dynamicArrayUsage);

    //actually call the functions
    //    sdpa::modules::Module::data_t params;
    //    test_dynamicArrayUsage(params);

  }

}
