#ifndef _PC_H_
#define _PC_H_

#ifdef __GNUC__
//#  define DEPRECATE_API(msg) __attribute__((deprecated(msg)))
#  define DEPRECATE_API(msg) __attribute__((deprecated))
#else
#  define DEPRECATE_API(msg)
#endif

#include <cstring>

typedef unsigned long fvmAllocHandle_t;
typedef unsigned long fvmSize_t;
typedef unsigned long fvmOffset_t;
typedef unsigned long fvmShmemOffset_t;
typedef int           fvmCommHandle_t;

typedef enum
  {
    COMM_HANDLE_ERROR_SHMEM_BOUNDARY = -11,

    COMM_HANDLE_ERROR_INVALID_HANDLE,
    COMM_HANDLE_ERROR_INVALID_SCRATCH_HANDLE,
    COMM_HANDLE_ERROR_INVALID_SIZE,
    COMM_HANDLE_ERROR_ARENA_UNKNOWN,

    /* error states */
    COMM_HANDLE_ERROR_TOO_MANY,
    COMM_HANDLE_ERROR_HANDLE_UNKNOWN,
    COMM_HANDLE_ERROR_ACK_FAILED,
    COMM_HANDLE_ERROR_SCRATCH_SIZE_TOO_SMALL,
    COMM_HANDLE_ERROR_SIZE_NOT_MATCH,
    COMM_HANDLE_ERROR,
    COMM_HANDLE_FREE = 0, // 0
    COMM_HANDLE_NOT_FINISHED,
    COMM_HANDLE_OK

  } fvmCommHandleState_t;

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

int fvmConnect(fvm_pc_config_t) DEPRECATE_API("please switch to the gpi-space pc api");
int fvmLeave() DEPRECATE_API("please switch to the gpi-space pc api");

int fvmGetRank() DEPRECATE_API("please switch to the gpi-space pc api");
int fvmGetNodeCount() DEPRECATE_API("please switch to the gpi-space pc api");

fvmSize_t fvmGetShmemSize() DEPRECATE_API("please switch to the gpi-space pc api");

fvmAllocHandle_t fvmGlobalAlloc(fvmSize_t size) DEPRECATE_API("please switch to the gpi-space pc api");
fvmAllocHandle_t fvmGlobalAlloc(fvmSize_t size, const char *name) DEPRECATE_API("please switch to the gpi-space pc api");
int fvmGlobalFree(fvmAllocHandle_t ptr) DEPRECATE_API("please switch to the gpi-space pc api");

fvmAllocHandle_t fvmLocalAlloc(fvmSize_t size) DEPRECATE_API("please switch to the gpi-space pc api");
fvmAllocHandle_t fvmLocalAlloc(fvmSize_t size, const char *) DEPRECATE_API("please switch to the gpi-space pc api");
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
const char *fvmGetShmemName() DEPRECATE_API("please switch to the gpi-space pc api");

#endif

