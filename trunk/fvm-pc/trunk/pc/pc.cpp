#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/msg.h>
#ifdef SHMEM
#include <sys/shm.h>
#endif

#include "pc.hpp"

//msgqueue
int pcQueueID; 
key_t pcQueueKey;

//shmem vars
#ifdef SHMEM
int pcShmid;
key_t pcShmKey;
void * pcShm;
#endif


// ------ Internal functions for Process Container (not to be used by appllication)
int doRequest(fvmRequest_t op_request)
{

  msgQueueMsg_t msg;
  msg.mtype = REQUESTMSG;
  msg.request = op_request;

#ifdef DEBUGMSG
  printf("PC: Sending msg on queue %d type 2 (doRequest)\n", pcQueueID);
#endif
	
  if(msgsnd(pcQueueID, &msg, sizeof(msgQueueMsg_t), 0) < 0){
    perror("Error answering request");
    return -1;
  }

  return 0;
}

static int getAck()
{
  msgq_ack_t msg;

#ifdef DEBUGMSG
  printf("PC: Receiving  msg on queue %d type 4 (doRequest)\n", pcQueueID);
#endif

  if((msgrcv(pcQueueID, &msg, sizeof(msgq_ack_t), ACKMSG, 0)) == -1){
    perror("get ack failed");
    return (-1);
  }

  return msg.ret;
}


//-------------------- Interface for Process Container -----------------------
int fvmConnect(fvm_pc_config_t config)
{
  int ret=0;
  msgQueueMsg_t msg;

  //create queue
  pcQueueKey = ftok(config.msqfile,'b');
  if(pcQueueKey == -1)
    return -1;
#ifdef DEBUG
  printf("Connecting to fvm with key %d\n",pcQueueKey);
#endif


  if((pcQueueID = msgget(pcQueueKey, 0666)) == -1){
    perror("msgget failed");
    return -1;
  }

  //tell fvm (parent) we are here
  msg.mtype = 1;
  msg.request.op = START;

#ifdef DEBUGMSG
  printf("PC:Sending  msg on queue %d type 1 (fvmConnect)\n",pcQueueID);
#endif

  if(msgsnd(pcQueueID,&msg,sizeof(msgQueueMsg_t),0) < 0){
    perror("Sending OK");
    return (-1);
  }

  //establish connection with shared component (shmem, qp,...)
#ifdef SHMEM

	
  //Locate the segment.
  if((pcShmKey = ftok(config.shmemfile, 'R')) == -1){
    perror("connecto fvm:ftok");
    return -1;
  }
	 
  if ((pcShmid = shmget(pcShmKey, config.shmemsize, IPC_CREAT | 0666)) < 0) {
    perror("connect to fvm:shmget");
    return -1;
  }

  //Now we attach the segment to our data space.
  if ((pcShm = shmat(pcShmid, NULL, 0)) == (char *) -1) {
    perror("shmat");
    return -1;
  }

#else //qp

#endif

  return ret;
}

int fvmLeave()
{
  fvmRequest_t op_request; //just for the leave

  /* just detach from the shmem, the rest should be done by the
     FVM */
#ifdef SHMEM
  shmdt(pcShm);
#endif
	 
  op_request.op = LEAVE;
  doRequest(op_request);

  return 0;
}


fvmAllocHandle_t fvmGlobalAlloc(size_t size)
{
  fvmAllocHandle_t ptr=0;

  msgQueueAllocMsg_t allocmsg;

  fvmRequest_t request;
  request.op = FGLOBALLOC;
  request.args.arg_size = size;

  if(doRequest(request)){
    perror("error doing request");
    return ptr;
  }

  //get result
#ifdef DEBUGMSG
  printf("PC: Receiving msg on queue %d type 3 (globalalloc)\n",pcQueueID);
#endif

  if((msgrcv(pcQueueID,&allocmsg,sizeof(msgQueueAllocMsg_t),ALLOCMSG,0)) == -1){
    perror("alloc failed");
    return 0;
  }

  ptr = allocmsg.handle;

  return ptr;
}

int fvmGlobalFree(fvmAllocHandle_t ptr)
{
	int ret;
	fvmRequest_t request;
	request.op = FGLOBALFREE;
	request.args.arg_allochandle = ptr;

	if(doRequest(request))
		perror("error doing request");

	ret =  getAck();

#ifdef DEBUGALLOC
      printf("PC: global free returns %d \n", ret);	
#endif
	return ret;
}


fvmAllocHandle_t fvmLocalAlloc(size_t size)
{
  fvmAllocHandle_t ptr=0;

  msgQueueAllocMsg_t allocmsg;

  fvmRequest_t request;
  request.op = FLOCALLOC;
  request.args.arg_size = size;

  if(doRequest(request)){
    perror("error doing request");
    return ptr;
  }


  //get result
#ifdef DEBUGMSG
  printf("PC: Receiving msg on queue %d type 3 (localalloc)\n",pcQueueID);
#endif

  if((msgrcv(pcQueueID,&allocmsg,sizeof(msgQueueAllocMsg_t),ALLOCMSG,0)) == -1){
    perror("alloc failed");
    return 0;
  }

  ptr = allocmsg.handle;

  return ptr;
}

int fvmLocalFree(fvmAllocHandle_t ptr)
{

  int ret;
  fvmRequest_t request;
  request.op = FLOCALFREE;
  request.args.arg_allochandle = ptr;

  if(doRequest(request))
    perror("error doing request");

  ret = getAck();

#ifdef DEBUGALLOC
      printf("PC: local free returns %d \n", ret);	
#endif
      return ret;

}

static fvmCommHandle_t fvmCommData(const fvmAllocHandle_t handle,
				   const fvmOffset_t fvmOffset,
				   const size_t size,
				   const fvmShmemOffset_t shmemOffset,
				   const fvmAllocHandle_t scratchHandle,
				   const fvmOperation_t op)
{
  fvmRequest_t request;
  request.op = op;
  request.args.arg_handle = handle;
  request.args.arg_fvmOffset = fvmOffset;
  request.args.arg_size = size;
  request.args.arg_shmOffset = shmemOffset;
  request.args.arg_scratchhandle = scratchHandle;

  if(doRequest(request)){
    perror("error doing request");
    return -1;
  }


  fvmCommHandle_t commhandle = (fvmCommHandle_t) getAck();

  return commhandle;
}
 
fvmCommHandle_t fvmGetGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const size_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle)
{

  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, scratchHandle, GETGLOBAL);

#ifdef DEBUGCOMM
  printf("PC: GetGlobal received handle %d\n",commhandle);
#endif
  
  return commhandle;
}

fvmCommHandle_t fvmPutGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const size_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle)
{

  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, scratchHandle, PUTGLOBAL);
  
#ifdef DEBUGCOMM
  printf("PC: PutGlobal received handle %d\n",commhandle);
#endif
  
  return commhandle;
  
}

fvmCommHandle_t fvmPutLocalData(const fvmAllocHandle_t handle,
				const fvmOffset_t fvmOffset,
				const size_t size,
				const fvmShmemOffset_t shmemOffset)
{

  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, 0, PUTLOCAL);
  
#ifdef DEBUGCOMM
  printf("PC: PutLocal received handle %d\n",commhandle);
#endif
  
  return commhandle;
}


fvmCommHandle_t fvmGetLocalData(const fvmAllocHandle_t handle,
				const fvmOffset_t fvmOffset,
				const size_t size,
				const fvmShmemOffset_t shmemOffset)
{
  
  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, 0, GETLOCAL);
  
#ifdef DEBUGCOMM
  printf("PC: GetLocal received handle %d\n",commhandle);
#endif
  
  return commhandle;

}

// wait on communication between fvm and pc
fvmCommHandleState_t waitComm(fvmCommHandle_t handle)
{
  fvmRequest_t request;
  request.op = WAITCOMM;
  request.args.arg_handle  = handle;
  
  if(doRequest(request))
    perror("error doing request");
  
  
  return (fvmCommHandleState_t) getAck();
  
}

void *fvmGetShmemPtr()
{
#ifdef SHMEM

#ifdef DEBUG
  printf("PC: return shmem pointer is %p\n", pcShm);
#endif

  return pcShm;
#else
  return 0;
#endif
}
