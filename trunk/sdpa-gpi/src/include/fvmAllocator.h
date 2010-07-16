#ifndef _FVM_ALLOCATOR_H_
#define _FVM_ALLOCATOR_H_

#include "fvmAllocatorTypes.h"

/* ************ Function definitions ****************** */
int fvmMMInit(void * ptr, size_t length, int rank, int nnodes,const char **hosts);

int fvmMMFinalize(void);

fvmAllocHandle_t fvmGlobalMMAlloc(fvmSize_t size);
int fvmGlobalMMFree(fvmAllocHandle_t handle);
fvmAllocHandle_t fvmLocalMMAlloc(fvmSize_t size);
int fvmLocalMMFree(fvmAllocHandle_t handle);

#endif
