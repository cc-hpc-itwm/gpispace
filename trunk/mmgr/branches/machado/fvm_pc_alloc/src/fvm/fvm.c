#include <string.h>

#include <sys/msg.h>

#ifdef SHMEM
#include <sys/shm.h>
#include <sys/types.h>
#endif

#include <dtmmgr.h>
#include <stack.h>


#include "fvm.h"
#include "fvmConfig.h"
#include "fvmSync.h"

#define MAXHANDLES 256

volatile fvmCommHandleState_t *handles;
volatile fvmCommHandle_t num_handles;
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
    [FGLOBALLOC] = "Global allocation",
    [FGLOBALFREE] = "Global deallocation",
    [FLOCALLOC] = "Local allocation",
    [FLOCALFREE] = "Local deallocation",
    [PUTGLOBAL] = "Put global data",
    [GETGLOBAL] = "Get global data",
    [PUTLOCAL] = "Put local data",
    [GETLOCAL] = "Get local data",
    [WAITCOMM] = "Wait communication",
    [LEAVE] = "Leave"
  };
	
  if(op < FGLOBALLOC || op > LEAVE)
    return "unknown";

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
  usedHandlesStack = (stack_t *) malloc(sizeof(stack_t));

  /* start array for holding handles */
  handles = (fvmCommHandleState_t *) calloc(MAXHANDLES + 1, sizeof(fvmCommHandleState_t));

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


  ret = shmctl(shmid, IPC_RMID, &buf);
  //  free(shm); 
  if(ret != 0)
    perror("removing shmem");

  ret += msgctl(fvmQueueID, IPC_RMID, &msq_status);
  if(ret != 0)
    perror("removing msgq");

  free(usedHandlesStack);
  free((void *)handles);

  return ret;
#endif
}



int fvmWait4PC(configFile_t config)
{
  int ret = 0;

  /* wait for a msg to start */
  msgQueueMsg_t msg;

#ifndef NDEBUGCMSG
  pv4d_printf("FVM:Receiving  msg on queue %d type 1 (waitPC)\n",fvmQueueID);
#endif


  if((msgrcv(fvmQueueID,&msg,sizeof(msgQueueMsg_t),STARTMSG,0)) == -1)
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
  shm = malloc(config.shmemsize);
  if(shm == NULL)
    {
      printf("failed to allocate buffer\n");
      ret = -1;
    }
  

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

  return ret;
}

/* ---------- Interface for Process Container requests (fvm internal side) */
static fvmAllocHandle_t fvmGlobalAllocInternal(unsigned int size)
{

#ifndef NDEBUGALLOC
  pv4d_printf("FVM: Global Allocation of %u bytes\n",size);
#endif

  return fvmGlobalMMAlloc(size);
}

static fvmAllocHandle_t fvmLocalAllocInternal(unsigned int size)
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

  if(msgsnd(fvmQueueID, &msg, sizeof(msgq_ack_t),0) < 0)
    {
      perror("Sending ACK");
      return (-1);
    }

  return 0;
	
}


static fvmCommHandleState_t fvmGlobalCommInternal(const fvmAllocHandle_t handle,
						  const fvmOffset_t fvmOffset,
						  const size_t transferSize,
						  const fvmShmemOffset_t shmemOffset,
						  const fvmAllocHandle_t scratchHandle,
						  const fvmOperation_t op)
{

  Offset_t handleOffset;
  MemSize_t handleSize;
  Offset_t scratchOffset;
  MemSize_t scratchSize;
  Arena_t arena;

  unsigned int offsetRank;
  fvmShmemOffset_t shmemInitialOffset;
  fvmOffset_t fvmInitialOffset;
  fvmOffset_t fvmInitialOffset_Locally;
  void * fvmAddress;

  size_t globalSize;	    
  size_t rest = 0;
  size_t currentTransferSize = 0;
  size_t availableSpace;
  fvmCommHandle_t *freeHandle;


  fvmAddress = getDmaMemPtrVM();

  if (!isCommAllowed())
    waitCommAllowed();

  /* update number of worked handles */
  /* see if there are free handles */
  freeHandle = (fvmCommHandle_t *) malloc(sizeof(fvmCommHandle_t));
  if(!stackPop(freeHandle, usedHandlesStack))
    {
      ++num_handles;
      //if we exhausted the max number of handles
      if(num_handles == MAXHANDLES)
	{
	  *freeHandle = num_handles;
	  handles[*freeHandle] = COMM_HANDLE_ERROR_TOO_MANY;  
	  num_handles = MAXHANDLES;
	  sendAck(*freeHandle);
	  goto out;
	}      
      else
	*freeHandle = num_handles;
    }    

#ifndef NDEBUGCOMM
  pv4d_printf("FVM: Global Comm: new handle for operation is %d\n", *freeHandle);
#endif

  handles[*freeHandle] = COMM_HANDLE_NOT_FINISHED; /* invalidate while we're not finished */
  
  /* send ack with communication handle straight away as we don't want to block */
  /* after this point, if any error occurs it will be saved as the handle state */
  if(sendAck(*freeHandle) < 0)
    {
      handles[*freeHandle]= COMM_HANDLE_ERROR_ACK_FAILED;
      goto out;
    }
  

#ifndef NBOUNDCHECK 
  if (transferSize + shmemOffset > configuration.shmemsize)
    {
      handles[*freeHandle] = COMM_HANDLE_ERROR_SHMEM_BOUNDARY;
      goto out;
    }
#endif 


#ifndef NDEBUGALLOC

  if (handle <= 0 )
    {
      handles[*freeHandle] = COMM_HANDLE_ERROR_INVALID_HANDLE;
      goto out;
    }
  if (scratchHandle <= 0 && (op == PUTGLOBAL || op == GETGLOBAL))
    {
      handles[*freeHandle] = COMM_HANDLE_ERROR_INVALID_SCRATCH_HANDLE;
      goto out;
    }
	
  if (transferSize == 0)
    {
      handles[*freeHandle] = COMM_HANDLE_ERROR_INVALID_SIZE;
      goto out;
    }
#endif


  if( op == PUTGLOBAL || op == GETGLOBAL)
    {
      arena = ARENA_GLOBAL;
      if( (dtmmgr_offset_size(dtmmgr, handle, ARENA_GLOBAL, &handleOffset, &handleSize)) != RET_SUCCESS)
	{	
	  handles[*freeHandle] = COMM_HANDLE_ERROR_HANDLE_UNKNOWN;
	  goto out;
	}      
    }
  else if (op == PUTLOCAL || op == GETLOCAL) 
    {
      arena = ARENA_LOCAL;
      if( (dtmmgr_offset_size(dtmmgr, handle, ARENA_LOCAL, &handleOffset, &handleSize)) != RET_SUCCESS)
	{
	  handles[*freeHandle] = COMM_HANDLE_ERROR_HANDLE_UNKNOWN;
	  goto out;
	}
    }
  else
    {
      /* we need to find if it is a global or local handle */
      if( (dtmmgr_offset_size(dtmmgr, handle, ARENA_GLOBAL, &handleOffset, &handleSize)) == RET_SUCCESS)
	arena = ARENA_GLOBAL;

      else if( (dtmmgr_offset_size(dtmmgr, handle, ARENA_LOCAL, &handleOffset, &handleSize)) == RET_SUCCESS)
	arena = ARENA_LOCAL;
	
      else 
	{
	  handles[*freeHandle] = COMM_HANDLE_ERROR_HANDLE_UNKNOWN;
	  goto out;
	}
    }

#ifndef NDEBUGCOMM 
  if(arena != ARENA_LOCAL && arena != ARENA_GLOBAL)
    {
      handles[*freeHandle] = COMM_HANDLE_ERROR_ARENA_UNKNOWN;
      goto out;
    }
#endif

#ifndef NDEBUGCOMM
  pv4d_printf("GLOBAL comm:handle %d starts with %d\n",*freeHandle,handles[*freeHandle]);
#endif


  if(arena == ARENA_GLOBAL)
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
	    
      /* scratch space should always be local */
      if(arena == ARENA_GLOBAL)
	dtmmgr_offset_size(dtmmgr, scratchHandle, ARENA_LOCAL, &scratchOffset,&scratchSize);

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
				  *freeHandle,
				  currentTransferSize);
#endif

		      handles[*freeHandle]= COMM_HANDLE_ERROR_SCRATCH_SIZE_TOO_SMALL;
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
				  *freeHandle,
				  currentTransferSize);
#endif

		      handles[*freeHandle]= COMM_HANDLE_ERROR_SCRATCH_SIZE_TOO_SMALL;	  
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

      handles[*freeHandle] = COMM_HANDLE_OK;
    }
  else 
    {

      /* put handle in error state */
      handles[*freeHandle] = COMM_HANDLE_ERROR_SIZE_NOT_MATCH;
      goto out;
    }
 out:	
  return (handles[*freeHandle]);

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
	  sleep(0.5);
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
      
      if((msgrcv(fvmQueueID,&msg, sizeof(msgQueueMsg_t),REQUESTMSG,0)) == -1){
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
	  
	if(msgsnd(fvmQueueID,&allocmsg,sizeof(msgQueueAllocMsg_t),0) < 0)
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
	  
	  if(msgsnd(fvmQueueID,&allocmsg,sizeof(msgQueueAllocMsg_t),0) < 0)
	    perror("Error answering request");
	  break;
	}

      case FLOCALFREE:
	sendAck(fvmLocalFreeInternal(op_request.args.arg_allochandle));
	break;

      case PUTGLOBAL:
	/* we don't want to send ack from here => non-blocking op */
	fvmPutGlobalDataInternal(op_request.args.arg_handle,
				 op_request.args.arg_fvmOffset,
				 op_request.args.arg_size,
				 op_request.args.arg_shmOffset,
				 op_request.args.arg_scratchhandle);


	break;
      case GETGLOBAL:
	/* we don't want to send ack from here => non-blocking op */
	fvmGetGlobalDataInternal(op_request.args.arg_handle,
				 op_request.args.arg_fvmOffset,
				 op_request.args.arg_size,
				 op_request.args.arg_shmOffset,
				 op_request.args.arg_scratchhandle);
	break;

      case PUTLOCAL:
	/* we don't want to send ack from here => non-blocking op */
	fvmPutLocalDataInternal(op_request.args.arg_handle,
				 op_request.args.arg_fvmOffset,
				 op_request.args.arg_size,
				 op_request.args.arg_shmOffset);


	break;
      case GETLOCAL:
	/* we don't want to send ack from here => non-blocking op */
	fvmGetLocalDataInternal(op_request.args.arg_handle,
				 op_request.args.arg_fvmOffset,
				 op_request.args.arg_size,
				 op_request.args.arg_shmOffset);
	break;

      case WAITCOMM:
	waitCommInternal(op_request.args.arg_handle);
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

    } while(op_request.op != LEAVE);
 /* 	} while(i < 10); */
		

  return 0;

}
