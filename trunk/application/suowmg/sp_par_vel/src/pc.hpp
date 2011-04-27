#ifndef _PC_H_
#define _PC_H_

#ifdef __GNUC__
//#  define DEPRECATE_API(msg) __attribute__((deprecated(msg)))
#  define DEPRECATE_API(msg) __attribute__((deprecated))
#else
#  define DEPRECATE_API(msg)
#endif

#include <fvm/fvmAllocatorTypes.h>
#include <fvm/fvm_common.h>

#include <stdlib.h>

// #include <cstring>

// typedef struct fvmPcConfig {
//   static const size_t max_len = 1024;

//   fvmPcConfig () {}
//   fvmPcConfig (const char * path_to_msq_file, const char * path_to_shm_file, const fvmSize_t shmem_size, const fvmSize_t fvm_size)
//     : shmemsize (shmem_size)
//     , fvmsize (fvm_size)
//   {
//      if (path_to_msq_file)
//         strncpy (msqfile, path_to_msq_file, max_len);
//      if (path_to_shm_file)
//         strncpy (shmemfile, path_to_shm_file, max_len);
//   }
//   fvmSize_t shmemsize;
//   fvmSize_t fvmsize;
//   char msqfile[max_len];
//   char shmemfile[max_len];
// } fvm_pc_config_t;

// int fvmConnect(fvm_pc_config_t) DEPRECATE_API("please switch to the gpi-space pc api");
int fvmLeave() DEPRECATE_API("please switch to the gpi-space pc api");

int fvmGetRank() DEPRECATE_API("please switch to the gpi-space pc api");
int fvmGetNodeCount() DEPRECATE_API("please switch to the gpi-space pc api");

fvmSize_t fvmGetShmemSize() DEPRECATE_API("please switch to the gpi-space pc api");

fvmAllocHandle_t fvmGlobalAlloc(fvmSize_t size) DEPRECATE_API("please switch to the gpi-space pc api");
int fvmGlobalFree(fvmAllocHandle_t ptr) DEPRECATE_API("please switch to the gpi-space pc api");

fvmAllocHandle_t fvmLocalAlloc(fvmSize_t size) DEPRECATE_API("please switch to the gpi-space pc api");
int fvmLocalFree(fvmAllocHandle_t ptr) DEPRECATE_API("please switch to the gpi-space pc api");

fvmCommHandle_t fvmGetGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle) DEPRECATE_API("please switch to the gpi-space pc api");


fvmCommHandle_t fvmPutGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle) DEPRECATE_API("please switch to the gpi-space pc api");

fvmCommHandle_t fvmGetLocalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				 const fvmShmemOffset_t shmemOffset) DEPRECATE_API("please switch to the gpi-space pc api");

fvmCommHandle_t fvmPutLocalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				const fvmShmemOffset_t shmemOffset) DEPRECATE_API("please switch to the gpi-space pc api");



fvmCommHandleState_t waitComm(fvmCommHandle_t handle) DEPRECATE_API("please switch to the gpi-space pc api");

void *fvmGetShmemPtr() DEPRECATE_API("please switch to the gpi-space pc api");

#endif

