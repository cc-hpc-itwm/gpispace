#include <string.h>
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
  , FVM_ESRCH      = -5
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
	  cleanUp();
	  return FVM_EOUTOFMEM;
	}
	memset(pcShm, 0, pcShmSize);

    pcFVMSize = cfg.fvmsize;
    fprintf(stderr, "fvm-pc: allocating %lu bytes of VM\n", pcFVMSize);
    pcFVM = malloc(pcFVMSize);
	if (! pcFVM)
	{
	  fprintf(stderr, "fvm-pc: VM memory allocation failed!");
	  cleanUp();
	  return FVM_EOUTOFMEM;
	}
	memset(pcFVM, 0, pcFVMSize);

    fprintf(stderr, "fvm-pc: using %lu bytes of VM memory\n", pcFVMSize);

    dtmmgr_init (&dtmmgr, pcFVMSize, 2);
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

  fvmAllocHandle_t ptr= (++num_allocs);
  AllocReturn_t AllocReturn = dtmmgr_alloc( &dtmmgr, (Handle_t)ptr, (Arena_t)ARENA_GLOBAL, (Size_t)size);
  switch (AllocReturn)
  {
	case ALLOC_SUCCESS:
#ifndef NDEBUG
      fprintf(stderr, "fvm-pc: global alloc handle %lu -> %lu bytes\n", ptr, size);
	  dtmmgr_status (dtmmgr);
#endif
	  return ptr;
	case ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY:
#ifndef NDEBUG
      fprintf(stderr, "fvm-pc: global alloc failed: not enough contigiuous memory!\n");
	  dtmmgr_status (dtmmgr);
#endif
	  // FIXME: set errno!
	  return 0;
	default:
#ifndef NDEBUG
      fprintf(stderr, "fvm-pc: global alloc failed!\n");
	  dtmmgr_status (dtmmgr);
#endif
	  return 0;
  }
}

int fvmGlobalFree(fvmAllocHandle_t ptr)
{
#ifndef NDEBUG
      fprintf(stderr, "fvm-pc: global free: handle=%lu\n", ptr);
#endif

  switch (HandleReturn_t ret = dtmmgr_free (&dtmmgr, ptr, ARENA_GLOBAL))
  {
	case RET_SUCCESS:
#ifndef NDEBUG
	  dtmmgr_status (dtmmgr);
#endif
	  return 0;
	case RET_HANDLE_UNKNOWN:
	  return FVM_ESRCH;
	case RET_FAILURE:
	  return FVM_EGENERAL;
	default:
      fprintf(stderr, "fvm-pc: global free failed: unknown return code: %d\n", ret);
	  return FVM_EGENERAL;
  }
}

fvmAllocHandle_t fvmLocalAlloc(fvmSize_t size)
{
  static fvmAllocHandle_t num_allocs = 0;

  fvmAllocHandle_t ptr= (++num_allocs);
  AllocReturn_t AllocReturn = dtmmgr_alloc( &dtmmgr, (Handle_t)ptr, (Arena_t)ARENA_LOCAL, (Size_t)size);
  switch (AllocReturn)
  {
	case ALLOC_SUCCESS:
#ifndef NDEBUG
      fprintf(stderr, "fvm-pc: local alloc handle %lu -> %lu bytes\n", ptr, size);
	  dtmmgr_status (dtmmgr);
#endif
	  return ptr;
	case ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY:
#ifndef NDEBUG
      fprintf(stderr, "fvm-pc: local alloc failed: not enough contigiuous memory!\n");
	  dtmmgr_status (dtmmgr);
#endif
	  // FIXME: set errno!
	  return 0;
	default:
#ifndef NDEBUG
      fprintf(stderr, "fvm-pc: local alloc failed!\n");
	  dtmmgr_status (dtmmgr);
#endif
	  return 0;
  }
}

int fvmLocalFree(fvmAllocHandle_t ptr)
{
#ifndef NDEBUG
      fprintf(stderr, "fvm-pc: local free: handle=%lu\n", ptr);
#endif

  switch (HandleReturn_t ret = dtmmgr_free (&dtmmgr, ptr, ARENA_LOCAL))
  {
	case RET_SUCCESS:
#ifndef NDEBUG
	  dtmmgr_status (dtmmgr);
#endif
	  return 0;
	case RET_HANDLE_UNKNOWN:
	  return FVM_ESRCH;
	case RET_FAILURE:
	  return FVM_EGENERAL;
	default:
	  // FIXME: set errno instead!
      fprintf(stderr, "fvm-pc: local free failed: unknown return code: %d\n", ret);
	  return FVM_EGENERAL;
  }
}

static fvmCommHandle_t fvmCommData(const fvmAllocHandle_t handle,
				   const fvmOffset_t fvmOffset,
				   const fvmSize_t size,
				   const fvmShmemOffset_t shmemOffset,
				   const fvmAllocHandle_t /* scratchHandle */,
				   const fvmOperation_t op)
{
  Offset_t BaseOffset = -1;
  switch (op)
  {
	// FIXME: implement error handling (size mismatch)
	//        return COMM_HANDLE_ERROR_SHMEM_BOUNDARY etc as handles
	case GETGLOBAL:
	  dtmmgr_offset_size (dtmmgr, handle, ARENA_GLOBAL, &BaseOffset, NULL);
	  if ((BaseOffset + fvmOffset + size) > pcFVMSize)
	  {
#ifndef NDEBUG
		fprintf(stderr, "fvm-pc: GetGlobalData out of range: offset=%lu size=%lu fvm-size=%lu\n", (BaseOffset + fvmOffset), size, pcFVMSize);
#endif
		return COMM_HANDLE_ERROR_INVALID_SIZE;
	  }
	  memcpy((char*)(pcShm) + shmemOffset, (char*)(pcFVM) + BaseOffset + fvmOffset, size);
	  break;
	case PUTGLOBAL:
	  dtmmgr_offset_size (dtmmgr, handle, ARENA_GLOBAL, &BaseOffset, NULL);
	  if ((BaseOffset + fvmOffset + size) > pcFVMSize)
	  {
#ifndef NDEBUG
		fprintf(stderr, "fvm-pc: PutGlobalData out of range: offset=%lu size=%lu fvm-size=%lu\n", (BaseOffset + fvmOffset), size, pcFVMSize);
#endif
		return COMM_HANDLE_ERROR_INVALID_SIZE;
	  }
	  memcpy((char*)(pcFVM) + BaseOffset + fvmOffset, (char*)(pcShm) + shmemOffset, size);
	  break;
	case GETLOCAL:
	  dtmmgr_offset_size (dtmmgr, handle, ARENA_LOCAL, &BaseOffset, NULL);
	  if ((BaseOffset + fvmOffset + size) > pcShmSize)
	  {
#ifndef NDEBUG
		fprintf(stderr, "fvm-pc: GetLocalData out of range: offset=%lu size=%lu shmem-size=%lu\n", (BaseOffset + fvmOffset), size, pcShmSize);
#endif
		return COMM_HANDLE_ERROR_INVALID_SIZE;
	  }
	  memcpy((char*)(pcShm) + shmemOffset, (char*)(pcFVM) + BaseOffset + fvmOffset, size);
	  break;
	case PUTLOCAL:
	  dtmmgr_offset_size (dtmmgr, handle, ARENA_LOCAL, &BaseOffset, NULL);
	  if ((BaseOffset + fvmOffset + size) > pcShmSize)
	  {
#ifndef NDEBUG
		fprintf(stderr, "fvm-pc: PutLocalData out of range: offset=%lu size=%lu shmem-size=%lu\n", (BaseOffset + fvmOffset), size, pcShmSize);
#endif
		return COMM_HANDLE_ERROR_INVALID_SIZE;
	  }
	  memcpy((char*)(pcFVM) + BaseOffset + fvmOffset, (char*)(pcShm) + shmemOffset, size);
	  break;
	default:
	  fprintf(stderr, "fvm-pc: fvmCommData() unsupported comm-operation:%d\n", op);
	  return -1;
  }

  return COMM_HANDLE_OK;
}
 
fvmCommHandle_t fvmGetGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle)
{
  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, scratchHandle, GETGLOBAL);
  return commhandle;
}

fvmCommHandle_t fvmPutGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle)
{

  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, scratchHandle, PUTGLOBAL);
  return commhandle;
  
}

fvmCommHandle_t fvmPutLocalData(const fvmAllocHandle_t handle,
				const fvmOffset_t fvmOffset,
				const fvmSize_t size,
				const fvmShmemOffset_t shmemOffset)
{

  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, 0, PUTLOCAL);
  return commhandle;
}


fvmCommHandle_t fvmGetLocalData(const fvmAllocHandle_t handle,
				const fvmOffset_t fvmOffset,
				const fvmSize_t size,
				const fvmShmemOffset_t shmemOffset)
{
  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, 0, GETLOCAL);
  return commhandle;

}

// wait on communication between fvm and pc
fvmCommHandleState_t waitComm(fvmCommHandle_t handle)
{
  // this is done only in the fake module, the communication is always synchronous!
  // error codes are encoded in the handle's value.
  return (fvmCommHandleState_t)handle;
}

void *fvmGetShmemPtr()
{
#ifdef SHMEM
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
