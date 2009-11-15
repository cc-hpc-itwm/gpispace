#ifndef _PC_H_
#define _PC_H_

#include "fvmAllocator.h"
#include "fvm_common.h"
#include "fvmConfig.h"


int fvmConnect(configFile_t);
int fvmLeave();

int fvmGetRank();
int fvmGetNodeCount();

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
int doRequest(fvmRequest_t op_request);

#endif

