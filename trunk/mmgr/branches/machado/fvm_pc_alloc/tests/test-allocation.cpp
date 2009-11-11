#include <assert.h>
#include <Module.hpp>
#include <pc.h>

using namespace sdpa::modules;
using namespace std;
  
// Test allocations module
// each function is a test to be loaded by the process container

//test that should kick a defragmentation
void test_Defrag(Module::data_t &params)
{
  printf("*******************************************************************************\n");
  printf("********************************** Test Defrag ********************************\n");
  printf("*******************************************************************************\n");
  int ret;

  fvmAllocHandle_t allocH[8];
  fvmAllocHandle_t src;

  for(int i=0; i < 8; i++)
    {    
      allocH[i] = fvmGlobalAlloc((64 * 1024 * 1024));
      assert(allocH[i] > 0);
    }

  ret = fvmGlobalFree(allocH[1]);
  //  assert(ret == 0);
  
  ret = fvmGlobalFree(allocH[3]);
  //  assert(ret == 0);

  src = fvmGlobalAlloc((128 * 1024 * 1024));
  //  assert(src > 0);


  for(int i=0; i < 8; i++)
    {    
      ret = fvmGlobalFree( allocH[i] );
    }

  ret = fvmGlobalFree(src);
  //  assert(ret == 0);

}

//test for allocation while there's communication going on
void test_AllocComm(Module::data_t &params)
{

  printf("*******************************************************************************\n");
  printf("******************************* Test Alloc and comm ***************************\n");
  printf("*******************************************************************************\n");
  fvmCommHandleState_t stt;
  int ret;
  fvmAllocHandle_t src = fvmGlobalAlloc((256*1024*1024));
  //assert(src > 0);

  fvmAllocHandle_t scratch = fvmLocalAlloc(( 1 << 28));
  //assert(scratch > 0);

  fvmCommHandle_t commHandle = fvmPutGlobalData( src, 0,  ( 1 << 28 ), 0, scratch);
  //assert(commHandle > 0);

  fvmAllocHandle_t src1 = fvmGlobalAlloc(( 1 << 26 ));
  //assert(src > 0);

  stt =  waitComm(commHandle);
  //assert( stt == COMM_HANDLE_OK);

  ret = fvmGlobalFree(src);
  //assert(ret == 0);

  ret = fvmGlobalFree(src1);
  //assert(ret == 0);

  ret = fvmLocalFree(scratch);
  //assert(ret == 0);

}


// multi-threaded test


//exhaust global memory
void test_GlobalMemory(Module::data_t &params)
{
  printf("*******************************************************************************\n");
  printf("******************************** Test global memory ***************************\n");
  printf("*******************************************************************************\n");
  int ret;

  fvmAllocHandle_t src = fvmGlobalAlloc((128*1024*1024));
  //  assert(src > 0);

  fvmAllocHandle_t src1 = fvmGlobalAlloc((128*1024*1024));
  //assert(src1 > 0);

  fvmAllocHandle_t src2 = fvmGlobalAlloc((128*1024*1024));
  //assert(src2 > 0);

  fvmAllocHandle_t src3 = fvmGlobalAlloc((128*1024*1024));
  //assert(src3 > 0);

  fvmAllocHandle_t src4 = fvmGlobalAlloc((128*1024*1024));
  //assert(src4 == 0);

  ret = fvmGlobalFree(src);
  //assert(ret == 0);

  ret = fvmGlobalFree(src1);
  //assert(ret == 0);
  
  ret = fvmGlobalFree(src2);  
  //assert(ret == 0);

  ret =fvmGlobalFree(src3);
  //assert(ret == 0);

  ret =fvmGlobalFree(src4);
  //assert(ret != 0);

}


//test local memory
void test_LocalMemory(Module::data_t &params)
{
  printf("*******************************************************************************\n");
  printf("****************************** Test Local memory ******************************\n");
  printf("*******************************************************************************\n");
  int ret; 

  fvmAllocHandle_t src = fvmLocalAlloc((256*1024*1024));
  //assert(src > 0);

  fvmAllocHandle_t src1 = fvmLocalAlloc((256*1024*1024));
  //assert(src1 > 0);

  fvmAllocHandle_t src2 = fvmLocalAlloc((256*1024*1024));
  //assert(src2 > 0);

  fvmAllocHandle_t src3 = fvmLocalAlloc((256*1024*1024));
  //assert(src3 > 0);

  ret = fvmLocalFree(src);
  //assert(ret == 0); 

  ret = fvmLocalFree(src1);
  //assert(ret == 0);

  ret = fvmLocalFree(src2);
  //assert(ret == 0);

  ret = fvmLocalFree(src3);
  //assert(ret == 0);

}



// The init function will call all tests implemented
extern "C" {
  void sdpa_mod_init(Module *mod) {
    SDPA_REGISTER_FUN(mod, test_GlobalMemory);

    //actually call the functions
    sdpa::modules::Module::data_t params;
    test_GlobalMemory(params);
    test_LocalMemory(params);
    test_AllocComm(params);
    test_Defrag(params);
  }
}



