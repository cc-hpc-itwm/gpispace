
#ifndef _FVM_C_H_
#define _FVM_C_H_

typedef enum {
	STARTMSG = 1,
	REQUESTMSG,
	ALLOCMSG,
	ACKMSG
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
#ifndef NBOUNDCHECK
    COMM_HANDLE_ERROR_SHMEM_BOUNDARY = -10,
#endif

#ifndef NDEBUG
    COMM_HANDLE_ERROR_INVALID_HANDLE,
    COMM_HANDLE_ERROR_INVALID_SCRATCH_HANDLE,
    COMM_HANDLE_ERROR_INVALID_SIZE,
    COMM_HANDLE_ERROR_ARENA_UNKNOWN,
#endif
    /* error states */
    COMM_HANDLE_ERROR_TOO_MANY,
    COMM_HANDLE_ERROR_HANDLE_UNKNOWN,
    COMM_HANDLE_ERROR_ACK_FAILED,
    COMM_HANDLE_ERROR_SCRATCH_SIZE_TOO_SMALL,
    COMM_HANDLE_ERROR_SIZE_NOT_MATCH, 
    COMM_HANDLE_FREE, // 0
    COMM_HANDLE_NOT_FINISHED,
    COMM_HANDLE_OK

  } fvmCommHandleState_t;

/* TODO: maybe an different struct for the put/get to make the normal struct smaller? */
typedef struct fvmRequestArgs
{
  fvmOffset_t arg_fvmOffset;
  fvmShmemOffset_t arg_shmOffset;
  unsigned int arg_size;
  fvmCommHandle_t arg_handle;
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

typedef struct msgq_ack {
  long mtype;
  int ret;
} msgq_ack_t;




#endif
