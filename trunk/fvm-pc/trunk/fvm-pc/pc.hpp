#ifndef _PC_H_
#define _PC_H_

#include <fvm/fvmAllocatorTypes.h>
#include <fvm/fvm_common.h>
#include <cstring>

typedef struct fvmPcConfig {
  static const size_t max_len = 1024;

  fvmPcConfig () {}
  fvmPcConfig (const char * path_to_msq_file, const char * path_to_shm_file, const fvmSize_t shmem_size, const fvmSize_t fvm_size)
    : shmemsize (shmem_size)
    , fvmsize (fvm_size)
  {
     if (path_to_msq_file)
        strncpy (msqfile, path_to_msq_file, max_len);
     if (path_to_shm_file)
        strncpy (shmemfile, path_to_shm_file, max_len);
  }
  fvmSize_t shmemsize;
  fvmSize_t fvmsize;
  char msqfile[max_len];
  char shmemfile[max_len];
} fvm_pc_config_t;

int fvmConnect(fvm_pc_config_t);
int fvmLeave();

int fvmGetRank();
int fvmGetNodeCount();

fvmSize_t fvmGetShmemSize();

fvmAllocHandle_t fvmGlobalAlloc(fvmSize_t size);
int fvmGlobalFree(fvmAllocHandle_t ptr);

fvmAllocHandle_t fvmLocalAlloc(fvmSize_t size);
int fvmLocalFree(fvmAllocHandle_t ptr);

fvmCommHandle_t fvmGetGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle);


fvmCommHandle_t fvmPutGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle);

fvmCommHandle_t fvmGetLocalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				 const fvmShmemOffset_t shmemOffset);

fvmCommHandle_t fvmPutLocalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				const fvmShmemOffset_t shmemOffset);
				 
		 

fvmCommHandleState_t waitComm(fvmCommHandle_t handle);

void *fvmGetShmemPtr();

//for now is here for the LEAVE but should go away
//int doRequest(fvmRequest_t op_request);

#endif

