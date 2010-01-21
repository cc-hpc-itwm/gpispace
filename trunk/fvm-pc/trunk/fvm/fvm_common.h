
#ifndef _FVM_COMMON_H_
#define _FVM_COMMON_H_

#include <fvm/fvmAllocatorTypes.h>

typedef enum {
	STARTMSG = 1,
	REQUESTMSG,
	ALLOCMSG,
	ACKMSG,
	CONNECTMSG
} fvmMsgType_t;

/* #define STARTMSG 1 */
/* #define REQUESTMSG 2 */
/* #define ALLOCMSG 3 */
/* #define ACKMSG 4 */

typedef unsigned long fvmOffset_t;
typedef unsigned long fvmShmemOffset_t;


typedef enum {
  START,
  FGLOBALLOC,
  FLOCALLOC,
  FGLOBALFREE,
  FLOCALFREE,
  PUTGLOBAL,
  GETGLOBAL,
  PUTLOCAL,
  GETLOCAL,
  READDMA,
  WRITEDMA,
  WAITCOMM,
  LEAVE
} fvmOperation_t;

/* handle for communication */
typedef int fvmCommHandle_t;

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

/* TODO: maybe an different struct for the put/get to make the normal struct smaller? */
typedef struct fvmRequestArgs
{
  fvmOffset_t arg_fvmOffset;
  fvmShmemOffset_t arg_shmOffset;
  fvmSize_t arg_size;
  fvmCommHandle_t  arg_commhandle;
  fvmAllocHandle_t arg_allochandle;
  fvmAllocHandle_t arg_scratchhandle;
} fvmRequestArgs_t;

typedef struct fvmRequest
{
  fvmOperation_t op;
  fvmRequestArgs_t args;
} fvmRequest_t;


typedef struct msgQueueMsg {
  long mtype; /* always needed by msgqueue */
  fvmRequest_t request;
} msgQueueMsg_t;


typedef struct msgQueueAllocMsg {
  long mtype; /* always needed by msgqueue */
  fvmAllocHandle_t handle;
} msgQueueAllocMsg_t;

typedef struct msgQueueConnectMsg {
  long mtype; /* always needed by msgqueue */
  int rank;
  int nodecount;
} msgQueueConnectMsg_t;

typedef struct msgq_ack {
  long mtype;
  int ret;
} msgq_ack_t;

#endif
