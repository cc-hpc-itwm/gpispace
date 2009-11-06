#include <string>
#include <assert.h>

#include <Module.hpp>

#include <pc.h>
#include <Pv4dLogger.h>

using namespace sdpa::modules;
using namespace std;


// Test communication functionality

// transfer from own node only
void test_ownNode(Module::data_t &params)
{
  printf("*******************************************************************************\n");
  printf("------------------------- Test Communication own node -------------------------\n");
  printf("*******************************************************************************\n");

  fvmCommHandleState_t stt;
  int ret;
  int *intPtr;

  int numInts = ( 1 << 28 ) / sizeof(int);

  fvmShmemOffset_t shmOff = 0;
  fvmOffset_t fvmOff = 0;
  size_t transfer = 0;
  fvmCommHandle_t commh;

  fvmAllocHandle_t src = fvmGlobalAlloc(( 1 << 28 ));
  assert(src > 0);

  fvmAllocHandle_t src1 = fvmGlobalAlloc(( 1 << 28 ));
  assert(src1 > 0);

  fvmAllocHandle_t scratch = fvmLocalAlloc(( 1 << 28));
  assert(scratch > 0);

  fvmCommHandle_t commHandles[256];
  for(int i=0; i < 25; i++)
    commHandles[i] = fvmPutGlobalData( src, 0, 1048576, 0, scratch);

  for(int i=0; i< 25; i++)
    {
       stt = waitComm(commHandles[i]);
       assert( stt == COMM_HANDLE_OK);
    }


  ret = fvmGlobalFree(src);
  assert(ret == 0);

  ret = fvmLocalFree(scratch);
  assert(ret == 0);


  // try to work with some real data
  intPtr = (int *) fvmGetShmemPtr();
  printf("received ptr is %p\n", intPtr);

  src = fvmGlobalAlloc( ( 1 << 28));
  assert(src > 0);

  scratch = fvmLocalAlloc(( 1 << 28));
  assert(scratch > 0);

  for (int i=0; i < numInts; i++)
    intPtr[i] = i;

  transfer = numInts * sizeof(int);

  commh = fvmPutGlobalData(src, fvmOff, transfer, shmOff, scratch);
  stt = waitComm(commh);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    intPtr[i] = 0;

  commh = fvmGetGlobalData(src, fvmOff, transfer, shmOff,scratch);
  stt = waitComm(commh);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    {
      if( intPtr[ i ] != i)
	printf("value is wrong %d: %d\n",i, intPtr[ i ] );
    }

  //do the same with the other alloc handle
  for (int i=0; i < numInts; i++)
    intPtr[i] = i;

  transfer = numInts * sizeof(int);

  commh = fvmPutGlobalData(src1, fvmOff, transfer, shmOff, scratch);
  stt = waitComm(commh);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    intPtr[i] = 0;

  commh = fvmGetGlobalData(src1, fvmOff, transfer, shmOff,scratch);
  stt = waitComm(commh);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    {
      if( intPtr[ i ] != i)
	printf("value is wrong %d: %d\n",i, intPtr[ i ] );
    }



  ret = fvmGlobalFree(src);
  assert(ret == 0);

  ret = fvmGlobalFree(src1);
  assert(ret == 0);

  ret = fvmLocalFree(scratch);
  assert(ret == 0);

}

// transfer is remote
void test_remoteNode(Module::data_t &params)
{
  printf("*******************************************************************************\n");
  printf("------------------------ Test Communication remote node -----------------------\n");
  printf("*******************************************************************************\n");

  int *intPtr;
  size_t transfer;
  int numInts = ( 1 << 26 ) / sizeof(int);

  fvmShmemOffset_t shmOff = 0;
  fvmOffset_t fvmOff = ( 1 << 26);

  fvmCommHandleState_t stt;
  int ret;

  fvmAllocHandle_t src = fvmGlobalAlloc(( 1 << 26 ));
  assert(src > 0);

  fvmAllocHandle_t src1 = fvmGlobalAlloc(( 1 << 26 ));
  assert(src1 > 0);

  fvmAllocHandle_t scratch = fvmLocalAlloc(( 1 << 29));
  assert(scratch > 0);

  intPtr = (int *) fvmGetShmemPtr();
  printf("received ptr is %p\n", intPtr);

  for (int i=0; i < numInts; i++)
    intPtr[i] = i;

  transfer = numInts * sizeof(int);

  fvmCommHandle_t commHandle = fvmPutGlobalData( src, fvmOff, transfer, shmOff, scratch);
  stt =  waitComm(commHandle);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    intPtr[i] = 0;

  commHandle = fvmGetGlobalData(src, fvmOff, transfer, shmOff, scratch);
  stt = waitComm(commHandle);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    {
      if( intPtr[ i ] != i)
	printf("value is wrong %d: %d\n",i, intPtr[ i ] );
    }

  //with the other alloc handle
  for (int i=0; i < numInts; i++)
    intPtr[i] = i;

  transfer = numInts * sizeof(int);

  commHandle = fvmPutGlobalData( src1, fvmOff, transfer, shmOff, scratch);
  stt =  waitComm(commHandle);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    intPtr[i] = 0;

  commHandle = fvmGetGlobalData(src1, fvmOff, transfer, shmOff, scratch);
  stt = waitComm(commHandle);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    {
      if( intPtr[ i ] != i)
	printf("value is wrong %d: %d\n",i, intPtr[ i ] );
    }



  ret = fvmGlobalFree(src);
  assert(ret == 0);

  ret = fvmGlobalFree(src1);
  assert(ret == 0);

  ret = fvmLocalFree(scratch);
  assert(ret == 0);

}

// transfer starts at own node but spreads over nodes
void test_ownNodeSpread(Module::data_t &params)
{
  printf("*******************************************************************************\n");
  printf("------------------- Test Communication own node but spreads -------------------\n");
  printf("*******************************************************************************\n");

  int *intPtr;
  size_t transfer;
  int numInts = ( ( 1 << 26 ) * 3 ) / sizeof(int);

  fvmShmemOffset_t shmOff = 0;
  fvmOffset_t fvmOff = 0;

  fvmCommHandleState_t stt;
  int ret;

  fvmAllocHandle_t src = fvmGlobalAlloc(( 1 << 26 ));
  assert(src > 0);

  fvmAllocHandle_t src1 = fvmGlobalAlloc(( 1 << 26 ));
  assert(src > 0);

  fvmAllocHandle_t scratch = fvmLocalAlloc(( 1 << 29));
  assert(scratch > 0);

  intPtr = (int *) fvmGetShmemPtr();
  printf("received ptr is %p\n", intPtr);

  for (int i=0; i < numInts; i++)
    intPtr[i] = i;

  transfer = numInts * sizeof(int);

  fvmCommHandle_t commHandle = fvmPutGlobalData( src, fvmOff, transfer, shmOff, scratch);
  stt =  waitComm(commHandle);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    intPtr[i] = 0;

  commHandle = fvmGetGlobalData(src, fvmOff, transfer, shmOff, scratch);
  stt = waitComm(commHandle);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    {
      if( intPtr[ i ] != i)
	printf("value is wrong %d: %d\n",i, intPtr[ i ] );
    }


  //with the other alloc handle
  for (int i=0; i < numInts; i++)
    intPtr[i] = i;

  transfer = numInts * sizeof(int);

  commHandle = fvmPutGlobalData( src1, fvmOff, transfer, shmOff, scratch);
  stt =  waitComm(commHandle);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    intPtr[i] = 0;

  commHandle = fvmGetGlobalData(src1, fvmOff, transfer, shmOff, scratch);
  stt = waitComm(commHandle);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    {
      if( intPtr[ i ] != i)
	printf("value is wrong %d: %d\n",i, intPtr[ i ] );
    }


  ret = fvmGlobalFree(src);
  assert(ret == 0);

  ret = fvmGlobalFree(src1);
  assert(ret == 0);

  ret = fvmLocalFree(scratch);
  assert(ret == 0);

}

// transfer is remote but spreads over nodes
void test_remoteNodeSpread(Module::data_t &params)
{
  printf("*******************************************************************************\n");
  printf("------------------- Test Communication remote node but spreads ----------------\n");
  printf("*******************************************************************************\n");
  
  int *intPtr;
  size_t transfer;
  int numInts = ( ( 1 << 26 ) * 3 ) / sizeof(int);

  fvmShmemOffset_t shmOff = 0;
  fvmOffset_t fvmOff = ( 1 << 26 );

  fvmCommHandleState_t stt;
  int ret;

  fvmAllocHandle_t src = fvmGlobalAlloc(( 1 << 26 ));
  assert(src > 0);

  fvmAllocHandle_t scratch = fvmLocalAlloc(( 1 << 29));
  assert(scratch > 0);

  intPtr = (int *) fvmGetShmemPtr();
  printf("received ptr is %p\n", intPtr);

  for (int i=0; i < numInts; i++)
    intPtr[i] = i;

  transfer = numInts * sizeof(int);

  fvmCommHandle_t commHandle = fvmPutGlobalData( src, fvmOff, transfer, shmOff, scratch);
  stt =  waitComm(commHandle);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    intPtr[i] = 0;

  commHandle = fvmGetGlobalData(src, fvmOff, transfer, shmOff, scratch);
  stt = waitComm(commHandle);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    {
      if( intPtr[ i ] != i)
	printf("value is wrong %d: %d\n",i, intPtr[ i ] );
    }

  ret = fvmGlobalFree(src);
  assert(ret == 0);

  ret = fvmLocalFree(scratch);
  assert(ret == 0);

}

//test local communication
void test_localcomm(Module::data_t &params)
{

  printf("*******************************************************************************\n");
  printf("---------------------- Test Local Communication --------------------------------\n");
  printf("*******************************************************************************\n");

  fvmCommHandleState_t stt;
  int ret;
  int *intPtr;
  char *bytePtr;

  int numInts = ( 1 << 27 ) / sizeof(int);

  fvmShmemOffset_t shmOff = 0;
  fvmOffset_t fvmOff = 0;
  size_t transfer = 0;

  fvmAllocHandle_t src = fvmLocalAlloc(( 1 << 28 ));
  assert(src > 0);

  fvmCommHandle_t commh = fvmPutLocalData(src, 0, (1 << 27), 0);

  fvmCommHandle_t commh1 = fvmGetLocalData(src, (1 << 20), (1 << 10), (1 << 20));

  stt = waitComm(commh);
  assert( stt == COMM_HANDLE_OK);
  
  stt = waitComm(commh1);
  assert( stt == COMM_HANDLE_OK);

  ret = fvmLocalFree(src);
  assert( ret == 0);

  intPtr = (int *) fvmGetShmemPtr();
  printf("received ptr is %p\n", intPtr);

  bytePtr = (char *) fvmGetShmemPtr();
  printf("received ptr is %p\n", bytePtr);

  // try to work with some real data
  src = fvmLocalAlloc((256*1024*1024));
  assert(src > 0);

  for (int i=0; i < numInts; i++)
    intPtr[i] = i;

//   for (int i=0; i < numInts; i++)
//     printf("%d ", intPtr[i] );

  transfer = numInts * sizeof(int);

  commh = fvmPutLocalData(src, fvmOff, transfer, shmOff);
  stt = waitComm(commh);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    intPtr[i] = 0;

  commh = fvmGetLocalData(src, 0, transfer, shmOff);
  stt = waitComm(commh);
  assert( stt == COMM_HANDLE_OK);

  for (int i=0; i < numInts; i++)
    {
      if( intPtr[ i ] != i)
	printf("value is wrong %d: %d\n",i, intPtr[ i ] );
    }

  ret = fvmLocalFree(src);
  assert(ret == 0);

}

// The init function will call all tests implemented
extern "C" {
  void sdpa_mod_init(Module *mod) {
    SDPA_REGISTER_FUN(mod, test_ownNode);

    //actually call the functions
    sdpa::modules::Module::data_t params;
    test_ownNode(params);
    test_remoteNode(params);
    test_ownNodeSpread(params);
    test_remoteNodeSpread(params);
    test_localcomm(params);
  }
}
