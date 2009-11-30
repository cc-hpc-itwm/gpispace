#include <stdlib.h>
#include <stdio.h>
#include <dtmmgr.h>

// #include <unistd.h>
// #include <errno.h>
// #include <sys/msg.h>

#include "pc.hpp"
#if ! defined(MAX_SHMEM_SIZE)
#error "MAX_SHMEM_SIZE must be defined!"
#endif
#if ! defined(MAX_FVM_SIZE)
#error "MAX_FVM_SIZE must be defined!"
#endif

// global variables
static void * pcShm(0);
static fvmSize_t pcShmSize(0);

static void * pcFVM(0);
static fvmSize_t pcFVMSize(0);

static DTmmgr_t dtmmgr(NULL);

enum FVM_PC_API_ERRORS
{
	FVM_EGENERAL   = -1
  , FVM_EINVALSZ   = -2
  , FVM_EOUTOFMEM  = -3
  , FVM_ETOOLARGE  = -4
};

static void cleanUp()
{
  if (pcShm)
  {
	fprintf (stderr, "fvm-pc: removing shared memory segment: %p\n", pcShm);
	free(pcShm); pcShm = NULL;
	pcShmSize = 0;
  }
  if (pcFVM)
  {
	fprintf (stderr, "fvm-pc: removing FVM segment: %p\n", pcFVM);
	free(pcFVM); pcFVM = NULL;
	pcFVMSize = 0;
  }

  if (dtmmgr)
  {
    fprintf (stderr, "fvm-pc: finalizing mmgr: %p %p\n", &dtmmgr, dtmmgr);
    Size_t Bytes = dtmmgr_finalize (&dtmmgr);
	fprintf (stderr, "fvm-pc: mmgr remaining bytes = %lu" "\n", Bytes);
  }
}

// ------ Internal functions for Process Container (not to be used by appllication)
//static int doRequest(fvmRequest_t /* op_request */ ) { return 0; }

//-------------------- Interface for Process Container -----------------------
int fvmConnect(fvm_pc_config_t cfg)
{
  if (!cfg.shmemsize)
  {
    fprintf(stderr, "fvm-pc: shared-memory size (0) is illegal!\n");
    return FVM_EINVALSZ;
  }
  
  if (cfg.shmemsize > MAX_SHMEM_SIZE)
  {
    fprintf(stderr, "fvm-pc: sorry, the configured shared-memory size is too large for me! maximum: %lu, wanted: %lu\n", (fvmSize_t)(MAX_SHMEM_SIZE), cfg.shmemsize);
    return FVM_ETOOLARGE;
  }
  if (cfg.fvmsize > MAX_FVM_SIZE)
  {
    fprintf(stderr, "fvm-pc: sorry, the configured vm-memory size is too large for me! maximum: %lu, wanted: %lu\n", (fvmSize_t)(MAX_FVM_SIZE), cfg.fvmsize);
    return FVM_ETOOLARGE;
  }

  else
  {
    pcShmSize = cfg.shmemsize;
    fprintf(stderr, "fvm-pc: allocating %lu bytes of shared-memory\n", pcShmSize);
    pcShm = malloc(pcShmSize);
	if (! pcShm)
	{
	  fprintf(stderr, "fvm-pc: shared memory allocation failed!");
	  return FVM_EOUTOFMEM;
	}

    pcFVMSize = cfg.fvmsize;
    fprintf(stderr, "fvm-pc: allocating %lu bytes of VM\n", pcFVMSize);
    pcFVM = malloc(pcFVMSize);
	if (! pcFVM)
	{
	  fprintf(stderr, "fvm-pc: shared memory allocation failed!");
	  return FVM_EOUTOFMEM;
	}

    fprintf(stderr, "fvm-pc: using %lu bytes of VM memory\n", cfg.fvmsize);

    dtmmgr_init (&dtmmgr, cfg.fvmsize, 2);
	dtmmgr_info (dtmmgr);

    return 0;
  }
}

int fvmLeave()
{
  cleanUp();
  return 0;
}

fvmAllocHandle_t fvmGlobalAlloc(fvmSize_t size)
{
  static fvmAllocHandle_t num_allocs = 0;
  if (size > 0)
  {
    fvmAllocHandle_t ptr= (++num_allocs);

#ifndef NDEBUG
    fprintf(stderr, "fvm-pc: global alloc handle %lu -> %lu bytes\n", ptr, size);
#endif
    return ptr;
  }
  else
  {
    fprintf(stderr, "fvm-pc: global alloc failed: size 0 is invalid\n");
  }

  return 0;
}

int fvmGlobalFree(fvmAllocHandle_t ptr)
{
  if (ptr > 0)
  {
    return 0;
  }
  return -1;
}

fvmAllocHandle_t fvmLocalAlloc(fvmSize_t size)
{
  static fvmAllocHandle_t num_allocs = 0;
  if (size > 0)
  {
    fvmAllocHandle_t ptr= (++num_allocs);

#ifndef NDEBUG
    fprintf(stderr, "fvm-pc: local alloc handle %lu -> %lu bytes\n", ptr, size);
#endif
    return ptr;
  }
  else
  {
    fprintf(stderr, "fvm-pc: local alloc failed: size 0 is invalid\n");
  }

  return 0;
}

int fvmLocalFree(fvmAllocHandle_t ptr)
{
  if (ptr > 0)
  {
    return 0;
  }
  return -1;
}

static fvmCommHandle_t fvmCommData(const fvmAllocHandle_t handle,
				   const fvmOffset_t fvmOffset,
				   const fvmSize_t size,
				   const fvmShmemOffset_t shmemOffset,
				   const fvmAllocHandle_t scratchHandle,
				   const fvmOperation_t op)
{
  fvmRequest_t request;
  request.op = op;
  request.args.arg_handle = handle;
  request.args.arg_fvmOffset = fvmOffset;
  request.args.arg_size = size;
  request.args.arg_shmOffset = shmemOffset;
  request.args.arg_scratchhandle = scratchHandle;

  return 0;
}
 
fvmCommHandle_t fvmGetGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle)
{
  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, scratchHandle, GETGLOBAL);

#ifndef NDEBUGCOMM
  fprintf(stderr, "fvm-pc: GetGlobal received handle %d\n",commhandle);
#endif
  
  return commhandle;
}

fvmCommHandle_t fvmPutGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle)
{

  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, scratchHandle, PUTGLOBAL);
  
#ifndef NDEBUGCOMM
  fprintf(stderr, "fvm-pc: PutGlobal received handle %d\n",commhandle);
#endif
  
  return commhandle;
  
}

fvmCommHandle_t fvmPutLocalData(const fvmAllocHandle_t handle,
				const fvmOffset_t fvmOffset,
				const fvmSize_t size,
				const fvmShmemOffset_t shmemOffset)
{

  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, 0, PUTLOCAL);
  
#ifndef NDEBUGCOMM
  fprintf(stderr, "fvm-pc: PutLocal received handle %d\n",commhandle);
#endif
  
  return commhandle;
}


fvmCommHandle_t fvmGetLocalData(const fvmAllocHandle_t handle,
				const fvmOffset_t fvmOffset,
				const fvmSize_t size,
				const fvmShmemOffset_t shmemOffset)
{
  
  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, 0, GETLOCAL);
  
#ifndef NDEBUGCOMM
  fprintf(stderr, "fvm-pc: GetLocal received handle %d\n",commhandle);
#endif
  
  return commhandle;

}

// wait on communication between fvm and pc
fvmCommHandleState_t waitComm(fvmCommHandle_t ) { return COMM_HANDLE_OK; }

void *fvmGetShmemPtr()
{
#ifdef SHMEM

#ifndef NDEBUG
  fprintf(stderr, "fvm-pc: return shmem pointer is %p\n", pcShm);
#endif

  return pcShm;
#else
  return 0;
#endif
}

fvmSize_t fvmGetShmSize()
{
  return pcShmSize;
}

int fvmGetRank()
{
  return 0;
}
int fvmGetNodeCount()
{
  return 1;
}
