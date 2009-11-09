#ifndef _PC_H_
#define _PC_H_

#include <fvm/fvmAllocatorTypes.h>
#include <fvm/fvm_common.h>

typedef struct fvmPcConfig {
  size_t shmemsize;
  size_t fvmsize;
  char msqfile[1024];
  char shmemfile[1024];
} fvm_pc_config_t;

int fvmConnect(fvm_pc_config_t);
int fvmLeave();


fvmAllocHandle_t fvmGlobalAlloc(size_t size);
int fvmGlobalFree(fvmAllocHandle_t ptr);

fvmAllocHandle_t fvmLocalAlloc(size_t size);
int fvmLocalFree(fvmAllocHandle_t ptr);

fvmCommHandle_t fvmGetGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const size_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle);


fvmCommHandle_t fvmPutGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const size_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle);

fvmCommHandle_t fvmGetLocalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const size_t size,
				 const fvmShmemOffset_t shmemOffset);

fvmCommHandle_t fvmPutLocalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const size_t size,
				const fvmShmemOffset_t shmemOffset);
				 
		 

fvmCommHandleState_t waitComm(fvmCommHandle_t handle);

void *fvmGetShmemPtr();

//for now is here for the LEAVE but should go away
int doRequest(fvmRequest_t op_request);

#endif

