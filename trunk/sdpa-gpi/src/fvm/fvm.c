#include <string.h>
#include <unistd.h>

#include <sys/msg.h>

#ifdef SHMEM
#include <sys/shm.h>
#include <sys/types.h>
#endif

#include <mmgr/dtmmgr.h>
#include <stack.h>


#include "fvm.h"
#include "fvmConfig.h"
#include "fvmSync.h"

#define MAXHANDLES 256

fvmCommHandleState_t handles[MAXHANDLES+1];
fvmCommHandle_t num_handles;
stack_t *usedHandlesStack;
configFile_t configuration;

extern DTmmgr_t dtmmgr;

/* msgqueue */
int fvmQueueID;
key_t fvmQueueKey;


int myRank;

/* shmem vars */
#ifdef SHMEM
int shmid;
key_t key;
void *shm;
#endif

/*  ---------------- Utilities --------------------- */

const char *op2str(fvmOperation_t op)
{

  static const char * const fvm_op_str[] = {
    "Start operation"
    , "Global allocation"
    , "Local allocation"
    , "Global deallocation"
    , "Local deallocation"
    , "Put global data"
    , "Get global data"
    , "Put local data"
    , "Get local data"
    , "Read DMA"
    , "Write DMA"
    , "Wait communication"
    , "Leave"
  };

  if(op < FGLOBALLOC || op > LEAVE)
	return "fvm operation unknown";

  return fvm_op_str[op];
}


int fvmInit(configFile_t config)
{

  configuration = config;

  /* create queue */
  fvmQueueKey = ftok(config.msqfile,'b');
  if(fvmQueueKey == -1)
	return -1;
#ifndef NDEBUGMSG
  pv4d_printf("Starting queue  with key %d\n",fvmQueueKey);
#endif

  if((fvmQueueID = msgget(fvmQueueKey, IPC_CREAT | 0666)) == -1){
	perror("msgget failed");
	return -1;
  }

  /* initialize stack of used but free to use handles */
  //  usedHandlesStack = (stack_t *) calloc(1, sizeof(stack_t));
  usedHandlesStack = (stack_t *) malloc( sizeof(stack_t));

  usedHandlesStack->numItems = 0;
  usedHandlesStack->stackPtr = NULL;

  /* start array for holding handles */
  memset(handles, COMM_HANDLE_FREE, sizeof(handles));

  /* initialize number of handles */
  num_handles = 0;

  return 0;
}

int fvmLeave()
{
#ifdef SHMEM
  struct shmid_ds buf;
  struct msqid_ds msq_status;
  int ret=0;

  shmdt(shm);
  ret = shmctl(shmid, IPC_RMID, &buf);

  if(ret != 0)
	perror("removing shmem");

  ret += msgctl(fvmQueueID, IPC_RMID, &msq_status);
  if(ret != 0)
	perror("removing msgq");

  fvmCommHandle_t freeHandle;
  while(stackPop(&freeHandle, usedHandlesStack)) {}

  free(usedHandlesStack); usedHandlesStack = NULL;

  return ret;
#endif
}



int fvmWait4PC(configFile_t config)
{
  int ret = 0;

  /* wait for a msg to start */
  msgQueueMsg_t msg;
  msgQueueConnectMsg_t connectMsg;

#ifndef NDEBUGCMSG
  pv4d_printf("FVM:Receiving  msg on queue %d type 1 (waitPC)\n",fvmQueueID);
#endif


//  if((msgrcv(fvmQueueID, &msg, sizeof(msgQueueMsg_t), STARTMSG, 0)) == -1)
  if((msgrcv(fvmQueueID, &msg, sizeof(msgQueueMsg_t) - sizeof(long), STARTMSG, 0)) == -1)
  {
	perror("msg recv failed");
	return (-1);
  }

  if(msg.request.op != START)
  {
	perror("Wrong start msg");
	return (-1);
  }

#ifdef SHMEM

  key = ftok(config.shmemfile, 'R');

  //create the segment.
  if ((shmid = shmget(key, config.shmemsize, IPC_CREAT | 0666)) < 0)
  {
	perror("shmget");
	ret = -1;
  }

  /*  Now we attach the segment to our data space. */
  if ((shm = shmat(shmid, NULL, 0)) == (char *) -1)
  {
	perror("shmat");
	ret=-1;
  }

#else

#endif

  myRank = getRankVM();
  connectMsg.mtype = CONNECTMSG;
  connectMsg.rank = myRank;
  connectMsg.nodecount = getNodeCountVM();

//  if(msgsnd(fvmQueueID, &connectMsg, sizeof(msgQueueConnectMsg_t), 0) < 0){
    if(msgsnd(fvmQueueID, &connectMsg, sizeof(msgQueueConnectMsg_t) - sizeof(long), 0) < 0){
	perror("FVM: Sending connect msg");
	return (-1);
  }

  return ret;
}

/* ---------- Interface for Process Container requests (fvm internal side) */
static fvmAllocHandle_t fvmGlobalAllocInternal(fvmSize_t size)
{

#ifndef NDEBUGALLOC
  pv4d_printf("FVM: Global Allocation of %u bytes\n",size);
#endif

  return fvmGlobalMMAlloc(size);
}

static fvmAllocHandle_t fvmLocalAllocInternal(fvmSize_t size)
{

#ifndef NDEBUGALLOC
  pv4d_printf("FVM: Local Allocation of %u bytes\n",size);
#endif

  return fvmLocalMMAlloc(size);
}

static int fvmFreeInternal(fvmAllocHandle_t handle)
{
  if(handle > 0 )
  {
	return fvmGlobalMMFree(handle);
  }
  return -1;
}

static int fvmLocalFreeInternal(fvmAllocHandle_t handle)
{

  if(handle > 0 )
	return fvmLocalMMFree(handle);
  return -1;
}

static int sendAck(int ret)
{
  msgq_ack_t msg;
  msg.ret = ret;
  msg.mtype = ACKMSG;

#ifndef NDEBUGMSG
  pv4d_printf("FVM: Sending msg on queue %d type 4 (sendAck) \n", fvmQueueID);
#endif

//  if(msgsnd(fvmQueueID, &msg, sizeof(msgq_ack_t),0) < 0)
  if(msgsnd(fvmQueueID, &msg, sizeof(int),0) < 0)
  {
	perror("Sending ACK");
	return (-1);
  }

  return 0;

}

/* TODO: non-blocking communication */
/* If we have enough scratch space, we could request several pieces of remote data */
/* something like: */
/* 1- calculate  all the remote transfers (offset, node)  we need to do and save them in a data structure */
/* 2- issue each tranfer (readDMA) using its own queue (at most 8 outstanding requests) to the scratch */
/* 3- wait on one queue, one after the other */
/* 4- when the first is finished, memcpy in parallel with the other remote requests */
/* 5- repeat step 4 for next queue, until everything is done */
static fvmCommHandleState_t fvmGlobalCommInternal(const fvmAllocHandle_t handle,
	const fvmOffset_t fvmOffset,
	const size_t transferSize,
	const fvmShmemOffset_t shmemOffset,
	const fvmAllocHandle_t scratchHandle,
	const fvmOperation_t op)
{
#ifndef NDEBUGCOMM
  pv4d_printf("FVM: fvmGlobalCommInternal(hdl=%lu, off=%lu, sz=%u, soff=%lu, scr=%lu, op=%s)\n",handle,fvmOffset,transferSize,shmemOffset,scratchHandle,op2str(op));
#endif

  Offset_t handleOffset;
  MemSize_t handleSize;
  Offset_t scratchOffset;
  MemSize_t scratchSize;
  Arena_t arena;

  fvmSize_t offsetRank;
  fvmShmemOffset_t shmemInitialOffset;
  fvmOffset_t fvmInitialOffset;
  fvmOffset_t fvmInitialOffset_Locally;
  void * fvmAddress;

  size_t globalSize;
  size_t rest = 0;
  size_t currentTransferSize = 0;
  size_t availableSpace;
  fvmCommHandle_t freeHandle = -1;


  fvmAddress = getDmaMemPtrVM();

  if (!isCommAllowed())
	waitCommAllowed();

  /* update number of worked handles */
  /* see if there are free handles */
  if(!stackPop(&freeHandle, usedHandlesStack))
  {
	++num_handles;
	//if we exhausted the max number of handles
	if(num_handles == MAXHANDLES)
	{
	  freeHandle = num_handles;
	  handles[freeHandle] = COMM_HANDLE_ERROR_TOO_MANY;
	  num_handles = MAXHANDLES;
	  sendAck(freeHandle);
	  goto out;
	}
	else
	  freeHandle = num_handles;
  }

#ifndef NDEBUGCOMM
  pv4d_printf("FVM: Global Comm %s: new handle for operation is %d\n",op2str(op), freeHandle);
#endif

  handles[freeHandle] = COMM_HANDLE_NOT_FINISHED; /* invalidate while we're not finished */

  /* send ack with communication handle straight away as we don't want to block */
  /* after this point, if any error occurs it will be saved as the handle state */
  if(sendAck(freeHandle) < 0)
  {
	handles[freeHandle]= COMM_HANDLE_ERROR_ACK_FAILED;
	goto out;
  }


#ifndef NBOUNDCHECK
  if (transferSize + shmemOffset > configuration.shmemsize)
  {
	handles[freeHandle] = COMM_HANDLE_ERROR_SHMEM_BOUNDARY;
	goto out;
  }
#endif


#ifndef NDEBUGALLOC

  if (handle <= 0 )
  {
	handles[freeHandle] = COMM_HANDLE_ERROR_INVALID_HANDLE;
	goto out;
  }
  if (scratchHandle <= 0 && (op == PUTGLOBAL || op == GETGLOBAL))
  {
	handles[freeHandle] = COMM_HANDLE_ERROR_INVALID_SCRATCH_HANDLE;
	goto out;
  }

  if (transferSize == 0)
  {
	handles[freeHandle] = COMM_HANDLE_ERROR_INVALID_SIZE;
	goto out;
  }
#endif


  if( op == PUTGLOBAL || op == GETGLOBAL)
  {
	arena = ARENA_UP;
	HandleReturn_t dtmmgr_ret = dtmmgr_offset_size(dtmmgr, handle, ARENA_UP, &handleOffset, &handleSize);
	switch (dtmmgr_ret)
	{
	  case RET_SUCCESS:
		break;
	  case RET_FAILURE:
		handles[freeHandle] = COMM_HANDLE_ERROR;
#ifndef NDEBUGCOMM
  pv4d_printf("FVM: Global Comm %s: dtmmgr failed for handle=%u\n",op2str(op), handle);
#endif
		goto out;
	  case RET_HANDLE_UNKNOWN:
#ifndef NDEBUGCOMM
  pv4d_printf("FVM: Global Comm %s: dtmmgr does not recognize handle=%u\n",op2str(op), handle);
#endif
		handles[freeHandle] = COMM_HANDLE_ERROR_HANDLE_UNKNOWN;
		goto out;
	}
  }
  else if (op == PUTLOCAL || op == GETLOCAL)
  {
	arena = ARENA_DOWN;
	HandleReturn_t dtmmgr_ret = dtmmgr_offset_size(dtmmgr, handle, ARENA_DOWN, &handleOffset, &handleSize);
	switch (dtmmgr_ret)
	{
	  case RET_SUCCESS:
		break;
	  case RET_FAILURE:
		handles[freeHandle] = COMM_HANDLE_ERROR;
		goto out;
	  case RET_HANDLE_UNKNOWN:
		handles[freeHandle] = COMM_HANDLE_ERROR_HANDLE_UNKNOWN;
		goto out;
	}
  }
  else
  {
#ifndef NDEBUGCOMM
  pv4d_printf("FVM: Global Comm %s: no arena specified, trying to guess one.\n",op2str(op));
#endif
	HandleReturn_t dtmmgr_ret;
	if (handles[freeHandle] == COMM_HANDLE_NOT_FINISHED)
	{
	  dtmmgr_ret = dtmmgr_offset_size(dtmmgr, handle, ARENA_UP, &handleOffset, &handleSize);
	  switch (dtmmgr_ret)
	  {
		case RET_SUCCESS:
		  arena = ARENA_UP;
		  break;
		case RET_FAILURE:
		  handles[freeHandle] = COMM_HANDLE_ERROR;
		  break;
		case RET_HANDLE_UNKNOWN:
		  handles[freeHandle] = COMM_HANDLE_ERROR_HANDLE_UNKNOWN;
		  break;
	  }
	}

	// does not seem to be a global handle, check if it is a local one
	if (handles[freeHandle] != COMM_HANDLE_NOT_FINISHED)
	{
	  dtmmgr_ret = dtmmgr_offset_size(dtmmgr, handle, ARENA_DOWN, &handleOffset, &handleSize);
	  switch (dtmmgr_ret)
	  {
		case RET_SUCCESS:
		  arena = ARENA_DOWN;
		  break;
		case RET_FAILURE:
		  handles[freeHandle] = COMM_HANDLE_ERROR;
		  break;
		case RET_HANDLE_UNKNOWN:
		  handles[freeHandle] = COMM_HANDLE_ERROR_HANDLE_UNKNOWN;
		  break;
	  }
	}

	if (handles[freeHandle] != COMM_HANDLE_NOT_FINISHED)
	{
#ifndef NDEBUGCOMM
  pv4d_printf("FVM: Global Comm %s: failed to guess arena type.\n",op2str(op));
#endif
	  goto out;
	}
	else
	{
#ifndef NDEBUGCOMM
  pv4d_printf("FVM: Global Comm %s: guessed arena: %s\n",op2str(op), (arena==ARENA_DOWN ? "local" : "global") );
#endif
	}
  }

#ifndef NDEBUGCOMM
  if(arena != ARENA_DOWN && arena != ARENA_UP)
  {
	handles[freeHandle] = COMM_HANDLE_ERROR_ARENA_UNKNOWN;
	goto out;
  }
#endif

#ifndef NDEBUGCOMM
  pv4d_printf("GLOBAL comm:handle %d starts with %d\n",freeHandle,handles[freeHandle]);
#endif


  if(arena == ARENA_UP)
	globalSize = handleSize * getNodeCountVM();
  else
	globalSize = handleSize;

#ifndef NDEBUGCOMM
  pv4d_printf("GLOBAL comm: transferSize %lu  - globalsize %lu\n",
	  transferSize,
	  globalSize);
#endif

  if(fvmOffset + transferSize <= globalSize)
  {

	shmemInitialOffset = shmemOffset;
	fvmInitialOffset = fvmOffset;
	rest = transferSize;

	//only on global communication we need the scratch handle
	if( op == PUTGLOBAL || op == GETGLOBAL)
	{

	  if((dtmmgr_offset_size(dtmmgr, scratchHandle, ARENA_DOWN, &scratchOffset, &scratchSize)) != RET_SUCCESS)
	  {

		if((dtmmgr_offset_size(dtmmgr, scratchHandle, ARENA_UP, &scratchOffset, &scratchSize)) != RET_SUCCESS)
		{
		  handles[freeHandle] = COMM_HANDLE_ERROR_INVALID_SCRATCH_HANDLE;
		  goto out;
		}
	  }
	}

	/*       if(arena == ARENA_UP) */
	/* 	dtmmgr_offset_size(dtmmgr, scratchHandle, ARENA_DOWN, &scratchOffset, &scratchSize); */

	if( op == GETLOCAL)
	{
	  pv4d_printf("GETLOCAL\n");

	  memcpy(((char *)shm) + shmemInitialOffset, ((char *)fvmAddress) + fvmInitialOffset, transferSize);
	  handles[freeHandle] = COMM_HANDLE_OK;
	}

	else if( op == PUTLOCAL)
	{
	  pv4d_printf("PUTLOCAL\n");

	  memcpy(((char *)fvmAddress) + fvmInitialOffset, ((char *)shm) + shmemInitialOffset, transferSize);
	  handles[freeHandle] = COMM_HANDLE_OK;
	}

	else
	{
	  while (rest > 0)
	  {
		offsetRank = fvmInitialOffset / handleSize;

		fvmInitialOffset_Locally = handleOffset + (fvmInitialOffset % handleSize);

		availableSpace = handleOffset + handleSize - fvmInitialOffset_Locally;

		if(availableSpace > rest) /* we can copy everything */
		{
		  currentTransferSize = rest;
		}
		else /* only part of it */
		{
		  currentTransferSize = availableSpace;
		}

#ifndef NDEBUGCOMM
		pv4d_printf("FVM: Rank %u\n",
			offsetRank);
#endif

		/* if memory is on my memory part, just memcpy */
		if( (offsetRank) == myRank)
		{

#ifndef NDEBUGCOMM
		  pv4d_printf("FVM: Offset is on my rank. Doing memcpy shm %lu fvm %lu of %lu size\n",
			  (shmemInitialOffset),
			  (fvmInitialOffset_Locally),
			  currentTransferSize);
#endif

		  if(op == GETGLOBAL || op == GETLOCAL)
			memcpy(((char *)shm) + shmemInitialOffset, ((char *)fvmAddress) + fvmInitialOffset_Locally, currentTransferSize);
		  else
			memcpy(((char *)fvmAddress) + fvmInitialOffset_Locally, ((char *)shm) + shmemInitialOffset, currentTransferSize);
		}
		else /* is remote, use scratch space to do a dma and then memcpy it */
		{

		  if( op == GETGLOBAL)
		  {
			/* make sure the scratch handle has enough space */
			if(currentTransferSize > scratchSize)
			{
#ifndef NDEBUGCOMM
			  pv4d_printf("GETTGLOBAL Error: scratch space (%lu)  too small for handle %d (%lu) \n",
				  scratchSize,
				  freeHandle,
				  currentTransferSize);
#endif

			  handles[freeHandle]= COMM_HANDLE_ERROR_SCRATCH_SIZE_TOO_SMALL;
			  goto out;
			}

			readDmaVM(scratchOffset,
				handleOffset + (fvmInitialOffset % handleSize),
				currentTransferSize,
				(fvmInitialOffset / handleSize),
				VMQueue0);

			waitDmaVM(VMQueue0);

			memcpy( ((char *)shm) + shmemInitialOffset, ((char *)fvmAddress) + scratchOffset, currentTransferSize);
		  }

		  else
		  {

#ifndef NDEBUGCOMM
			pv4d_printf("FVM PUTGLOBAL: remote data %lu size with scratch size %lu\n",
				currentTransferSize,
				scratchSize);
#endif


			/* make sure the scratch handle has enough space */
			if(currentTransferSize > scratchSize)
			{
#ifndef NDEBUGCOMM
			  pv4d_printf("PUTGLOBAL Error: scratch space (%lu)  too small for handle %d (%lu) \n",
				  scratchSize,
				  freeHandle,
				  currentTransferSize);
#endif

			  handles[freeHandle]= COMM_HANDLE_ERROR_SCRATCH_SIZE_TOO_SMALL;
			  goto out;
			}


			memcpy( ((char *)fvmAddress) + scratchOffset,  ((char *)shm) + shmemInitialOffset, currentTransferSize);

			writeDmaVM(scratchOffset,
				handleOffset + (fvmInitialOffset % handleSize),
				currentTransferSize,
				(fvmInitialOffset / handleSize),
				VMQueue0);

			waitDmaVM(VMQueue0);


		  }
		}

		rest -= currentTransferSize;
		fvmInitialOffset += currentTransferSize;
		shmemInitialOffset += currentTransferSize;

	  }

	  handles[freeHandle] = COMM_HANDLE_OK;
	}
  }
  else
  {

	/* put handle in error state */
	handles[freeHandle] = COMM_HANDLE_ERROR_SIZE_NOT_MATCH;
	goto out;
  }
out:
  return (handles[freeHandle]);

}


static fvmCommHandleState_t fvmGetGlobalDataInternal(const fvmAllocHandle_t handle,
	const fvmOffset_t fvmOffset,
	const size_t transferSize,
	const fvmShmemOffset_t shmemOffset,
	const fvmAllocHandle_t scratchHandle)
{

  return fvmGlobalCommInternal( handle, fvmOffset, transferSize, shmemOffset, scratchHandle, GETGLOBAL);

}


static int fvmPutGlobalDataInternal (const fvmAllocHandle_t handle,
	const fvmOffset_t fvmOffset,
	const size_t transferSize,
	const fvmShmemOffset_t shmemOffset,
	const fvmAllocHandle_t scratchHandle)
{

  return fvmGlobalCommInternal( handle, fvmOffset, transferSize, shmemOffset, scratchHandle, PUTGLOBAL);

}


/* Local communication:  */
/* to be used when we know the handle is on the local arena */
/* to save lookup of the handle */
static int fvmGetLocalDataInternal(const fvmAllocHandle_t handle,
	const fvmOffset_t fvmOffset,
	const size_t transferSize,
	const fvmShmemOffset_t shmemOffset)

{
  return fvmGlobalCommInternal( handle, fvmOffset, transferSize, shmemOffset, 0, GETLOCAL);
}

static int fvmPutLocalDataInternal(const fvmAllocHandle_t handle,
	const fvmOffset_t fvmOffset,
	const size_t transferSize,
	const fvmShmemOffset_t shmemOffset)
{

  return fvmGlobalCommInternal( handle, fvmOffset, transferSize, shmemOffset, 0, PUTLOCAL);
}


static void waitCommInternal(fvmCommHandle_t handlecheck)
{
  //TODO: find best way for to wait for comm poll and passive
  int ret;

#ifndef NDEBUGCOMM
  pv4d_printf("FVM: WAITCOMM: handle %lu is  %d\n",handlecheck, handles[handlecheck]);
#endif


check_switch:
  switch(handles[handlecheck])
  {
#ifndef NBOUNDCHECK
	case COMM_HANDLE_ERROR_SHMEM_BOUNDARY:
#endif

#ifndef NDEBUGCOMM
	case COMM_HANDLE_ERROR_INVALID_HANDLE:
	case COMM_HANDLE_ERROR_INVALID_SCRATCH_HANDLE:
	case COMM_HANDLE_ERROR_INVALID_SIZE:
	case COMM_HANDLE_ERROR_ARENA_UNKNOWN:
#endif
	case COMM_HANDLE_ERROR_TOO_MANY:
	case COMM_HANDLE_ERROR_HANDLE_UNKNOWN:
	case COMM_HANDLE_ERROR_ACK_FAILED:
	case COMM_HANDLE_ERROR_SCRATCH_SIZE_TOO_SMALL:
	case COMM_HANDLE_ERROR_SIZE_NOT_MATCH:
	case COMM_HANDLE_FREE:
	  ret = handles[handlecheck];
	  break;

	case COMM_HANDLE_NOT_FINISHED:
	  while(handles[handlecheck] != COMM_HANDLE_OK)
	  {
		/* ugly... */
		usleep(500000);
		goto check_switch;
	  }
	  break;
	case COMM_HANDLE_OK:

	  {
		fvmCommHandle_t *handle_ptr = (fvmCommHandle_t *) malloc(sizeof(fvmCommHandle_t));
		ret = COMM_HANDLE_OK;
		/* invalidate handle to be re-used  */
		handles[handlecheck] = COMM_HANDLE_FREE;
		*handle_ptr = handlecheck;
		stackPush(usedHandlesStack, COMM_HANDLE_FREE, handle_ptr, sizeof(fvmCommHandle_t));

		break;
	  }
  }

  sendAck(ret);

}

int fvmListenRequests()
{
  fvmRequest_t op_request;
  msgQueueMsg_t msg;

  int i=0;

  do
  {

#ifndef NDEBUGMSG
	pv4d_printf("FVM: Receiving  msg on queue %d type 2 (listenRequests)\n",fvmQueueID);
#endif

//	if((msgrcv(fvmQueueID,&msg, sizeof(msgQueueMsg_t),REQUESTMSG,0)) == -1){
	if((msgrcv(fvmQueueID,&msg, sizeof(msgQueueMsg_t)-sizeof(long),REQUESTMSG,0)) == -1){
	  perror("msg recv failed");
	  return (-1);
	}

#ifndef NDEBUGMSG
	pv4d_printf("PARENT: Request was %s\n",op2str(msg.request.op));
#endif

	op_request = msg.request;

	switch(op_request.op){
	  case FGLOBALLOC:

		{
		  msgQueueAllocMsg_t allocmsg;
		  allocmsg.handle= fvmGlobalAllocInternal(op_request.args.arg_size);

		  /* in this case we need to answer back */
		  /* this might in a function of its own to use different mechanism */
		  allocmsg.mtype = ALLOCMSG;

#ifndef NDEBUGMSG
		  pv4d_printf("FVM: Sending  msg on queue %d type 3 (globalalloc)\n",fvmQueueID);
#endif

//		  if(msgsnd(fvmQueueID,&allocmsg,sizeof(msgQueueAllocMsg_t),0) < 0)
		  if(msgsnd(fvmQueueID,&allocmsg,sizeof(msgQueueAllocMsg_t) - sizeof(long),0) < 0)
			perror("Error answering request");
		  break;
		}

	  case FGLOBALFREE:
		sendAck(fvmFreeInternal(op_request.args.arg_allochandle));
		break;

	  case FLOCALLOC:
		{
		  msgQueueAllocMsg_t allocmsg;
		  allocmsg.handle= fvmLocalAllocInternal(op_request.args.arg_size);
		  allocmsg.mtype = ALLOCMSG;

#ifndef NDEBUGMSG
		  pv4d_printf("FVM: Sending  msg on queue %d type 3 (localalloc)\n",fvmQueueID);
#endif

		  if(msgsnd(fvmQueueID,&allocmsg,sizeof(msgQueueAllocMsg_t) - sizeof(long),0) < 0)
			perror("Error answering request");
		  break;
		}

	  case FLOCALFREE:
		sendAck(fvmLocalFreeInternal(op_request.args.arg_allochandle));
		break;

	  case PUTGLOBAL:
		/* we don't want to send ack from here => non-blocking op */
		fvmPutGlobalDataInternal(op_request.args.arg_allochandle,
			op_request.args.arg_fvmOffset,
			op_request.args.arg_size,
			op_request.args.arg_shmOffset,
			op_request.args.arg_scratchhandle);


		break;
	  case GETGLOBAL:
		/* we don't want to send ack from here => non-blocking op */
		fvmGetGlobalDataInternal(op_request.args.arg_allochandle,
			op_request.args.arg_fvmOffset,
			op_request.args.arg_size,
			op_request.args.arg_shmOffset,
			op_request.args.arg_scratchhandle);
		break;

	  case PUTLOCAL:
		/* we don't want to send ack from here => non-blocking op */
		fvmPutLocalDataInternal(op_request.args.arg_allochandle,
			op_request.args.arg_fvmOffset,
			op_request.args.arg_size,
			op_request.args.arg_shmOffset);


		break;
	  case GETLOCAL:
		/* we don't want to send ack from here => non-blocking op */
		fvmGetLocalDataInternal(op_request.args.arg_allochandle,
			op_request.args.arg_fvmOffset,
			op_request.args.arg_size,
			op_request.args.arg_shmOffset);
		break;

	  case WAITCOMM:
		waitCommInternal(op_request.args.arg_commhandle);
		break;

	  case LEAVE:
		if((fvmLeave()) == -1){
		  pv4d_printf("Error when leaving (cleaning resources).\n");
		}

		break;
	  default:
		printf("Unknown request\n");
	}

	i++;
        fflush (NULL);
  } while(op_request.op != LEAVE);
  /* 	} while(i < 10); */


return 0;

}
