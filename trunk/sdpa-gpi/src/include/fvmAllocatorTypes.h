#ifndef _FVM_ALLOCATOR_TYPES_H_
#define _FVM_ALLOCATOR_TYPES_H_

/* ************* Macros **************** */
#define IN_MB(X) (X / 1024 /1024)
#define ACK_OK 1
#define ACK_NOK 0

#define ALLOCATOR_PORT_REMOTE "14000" 
#define BACKLOG 10

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

#include <stdint.h>

typedef unsigned long fvmAllocHandle_t;
typedef uint64_t fvmSize_t;

typedef struct {
  fvmAllocType_t type;
  fvmSize_t size;
  int root;
  fvmAllocHandle_t handle;
} fvmAllocRequest_t;

#endif
