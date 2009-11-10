#ifndef _FVM_ALLOCATOR_H_
#define _FVM_ALLOCATOR_H_

/* ************* Macros **************** */
#define IN_MB(X) (X / 1024 /1024)
#define ACK_OK 1
#define ACK_NOK 0

#define ALLOCATOR_PORT_REMOTE "14000" 
#define BACKLOG 10

/* ************* type definitions *************** */
typedef struct fvmMemPointer
{
	void *fvmAddress;
	unsigned long long offset;
	unsigned int size;
}fvmMemPointer_t;

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

typedef struct {
  fvmAllocType_t type;
  unsigned long size;
  int root;
  fvmAllocHandle_t handle;
} fvmAllocRequest_t;

#endif
