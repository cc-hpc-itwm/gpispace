#ifndef _FVM_ALLOCATOR_H_
#define _FVM_ALLOCATOR_H_

/* ************* Macros **************** */
#define IN_MB(X) (X / 1024 /1024)
#define ACK_OK 1
#define ACK_NOK 0

#define ALLOCATOR_PORT_REMOTE "14000" 
#define BACKLOG 10

/* ************* type definitions *************** */
/* typedef struct fvmMemPointer */
/* { */
/* 	void *fvmAddress; */
/* 	unsigned long long offset; */
/* 	unsigned int size; */
/* }fvmMemPointer_t; */


typedef enum {
  QUERYGLOBALLOC,
  QUERYGLOBALLOCACK,
  ABORTGLOBALLOC,
  ABORTGLOBALLOCACK,
  COMMITGLOBALLOC,
  COMMITGLOBALLOCACK,
  COMMSTOPGLOBAL,
  COMMSTOPGLOBALACK,
  COMMITGLOBALLOCACKGLOBALLY,
  LOCALLOC,
  LOCALFREE,
  GLOBALFREE,
  GLOBALFREEACK
} fvmAllocType_t;

typedef unsigned long fvmAllocHandle_t;
typedef unsigned long fvmAllocSize_t;

typedef struct {
  fvmAllocType_t type;
  fvmAllocSize_t size;
  int root;
  fvmAllocHandle_t handle;
} fvmAllocRequest_t;



/* ************ Function definitions ****************** */
int fvmMMInit(void * ptr, size_t length, int rank, int nnodes,const char **hosts);

int fvmMMFinalize(void);

fvmAllocHandle_t fvmGlobalMMAlloc(fvmAllocSize_t size);
int fvmGlobalMMFree(fvmAllocHandle_t handle);
fvmAllocHandle_t fvmLocalMMAlloc(fvmAllocSize_t size);
int fvmLocalMMFree(fvmAllocHandle_t handle);

#endif
