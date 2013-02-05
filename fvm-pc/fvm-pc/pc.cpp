#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/msg.h>
#ifdef SHMEM
#include <sys/shm.h>
#endif

#include <fvm-pc/pc.hpp>

#include <we/loader/macros.hpp>

#include <fhg/util/show.hpp>

//msgqueue
static int pcQueueID;
static key_t pcQueueKey;

//shmem vars
#ifdef SHMEM
static int pcShmid;
static key_t pcShmKey;
static void * pcShm;
static fvmSize_t pcShmSize;
#endif

static int nodeRank;
static int nodeCount;

// ------ Internal functions for Process Container (not to be used by appllication)
static int doRequest(fvmRequest_t op_request)
{

  msgQueueMsg_t msg;
  msg.mtype = REQUESTMSG;
  msg.request = op_request;

#ifndef NDEBUGMSG
  printf("PC: Sending msg on queue %d type 2 (doRequest)\n", pcQueueID);
#endif

  // man msgsnd:
  //    int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);

  //        The application shall ensure that the argument msgp points
  //        to a  user-defined buffer that  contains first a  field of
  //        type long specifying  the type of the message,  and then a
  //        data portion that holds the data bytes of the message. The
  //        structure below  is an  example of what  this user-defined
  //        buffer might look like:


  //               struct mymsg {
  //                   long   mtype;       /* Message type. */
  //                   char   mtext[1];    /* Message text. */
  //               }

  //        The  structure member  mtype is  a non-zero  positive type
  //        long that can be used by the receiving process for message
  //        selection.

  //        The  structure member mtext  is any  text of  length msgsz
  //        bytes.  The   argument  msgsz  can  range  from   0  to  a
  //        system-imposed maximum.

  if(msgsnd(pcQueueID, &msg, sizeof(fvmRequest_t), 0) < 0){
    perror("Error answering request");
    return -1;
  }

  return 0;
}

static int getAck()
{
  msgq_ack_t msg;

#ifndef NDEBUGMSG
  printf("PC: Receiving  msg on queue %d type 4 (doRequest)\n", pcQueueID);
#endif

//   man msgrcv:
//      ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp,

//      struct mymsg {
//                   long    mtype;     /* Message type. */
//                   char    mtext[1];  /* Message text. */
//               }

//        The structure member mtype is the received message's type as
//        specified by the sending process.

//        The structure member mtext is the text of the message.

//        The  argument msgsz specifies  the size  in bytes  of mtext.
//        The received message shall be truncated to msgsz bytes if it
//        is larger than msgsz and (msgflg & MSG_NOERROR) is non-zero.
//        The  truncated part  of the  message  shall be  lost and  no
//        indication of  the truncation shall be given  to the calling
//        process.

  if((msgrcv(pcQueueID, &msg, sizeof(int), ACKMSG, 0)) == -1) {
    perror("get ack failed");
    return (-1);
  }

  return msg.ret;
}


//-------------------- Interface for Process Container -----------------------
int fvmConnect(fvm_pc_config_t config)
{
  // initialize globals
#ifdef SHMEM
  pcShmid = 0;
  pcShmKey = 0;
  pcShm = 0;
  pcShmSize = 0;
#endif

  int ret=0;
  msgQueueMsg_t msg;
  msgQueueConnectMsg_t connectMsg;

  //create queue
  pcQueueKey = ftok(config.msqfile,'b');
  if(pcQueueKey == -1)
  {
    perror("ftok failed");
    return -1;
  }
#ifndef NDEBUG
  printf("Connecting to fvm with key %d\n",pcQueueKey);
#endif


  if((pcQueueID = msgget(pcQueueKey, 0666)) == -1){
    perror("msgget failed");
    return -1;
  }

  //tell fvm (parent) we are here
  msg.mtype = 1;
  msg.request.op = START;

#ifndef NDEBUGMSG
  printf("PC:Sending  msg on queue %d type 1 (fvmConnect)\n",pcQueueID);
#endif

  if(msgsnd(pcQueueID,&msg,sizeof(msgQueueMsg_t) - sizeof(long),0) < 0){
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

  // set the size
  pcShmSize = config.shmemsize;

#else //qp

#endif

  if((msgrcv(pcQueueID, &connectMsg, sizeof(msgQueueConnectMsg_t) - sizeof(long), CONNECTMSG, 0)) == -1){
    perror("PC: error receiving connect msg");
    return 0;
  }

  nodeRank = connectMsg.rank;
  nodeCount = connectMsg.nodecount;

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

fvmAllocHandle_t fvmGlobalAlloc(fvmSize_t size, const char*)
{
  return fvmGlobalAlloc(size);
}

fvmAllocHandle_t fvmGlobalAlloc(fvmSize_t size)
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
#ifndef NDEBUGMSG
  printf("PC: Receiving msg on queue %d type 3 (globalalloc, size: %lu)\n", pcQueueID, size);
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

#ifndef NDEBUGALLOC
      printf("PC: global free returns %d \n", ret);
#endif
	return ret;
}


fvmAllocHandle_t fvmLocalAlloc(fvmSize_t size, const char*)
{
  return fvmLocalAlloc(size);
}

fvmAllocHandle_t fvmLocalAlloc(fvmSize_t size)
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
#ifndef NDEBUGMSG
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

#ifndef NDEBUGALLOC
      printf("PC: local free returns %d \n", ret);
#endif
      return ret;

}

static fvmCommHandle_t fvmCommData(const fvmAllocHandle_t handle,
				   const fvmOffset_t fvmOffset,
				   const fvmSize_t size,
				   const fvmShmemOffset_t shmemOffset,
				   const fvmAllocHandle_t scratchHandle,
				   const fvmOperation_t op)
{
  fvmRequest_t request;
  request.op = op;
  request.args.arg_allochandle = handle;
  request.args.arg_fvmOffset = fvmOffset;
  request.args.arg_size = size;
  request.args.arg_shmOffset = shmemOffset;
  request.args.arg_scratchhandle = scratchHandle;

//   fvmRequest_t request = {
//     op
//     , { fvmOffset
//         , shmemOffset
//         , size
//         , (fvmCommHandle_t)0
//         , handle
//         , scratchHandle
//     }
//   };

  if(doRequest(request)){
    perror("error doing request");
    return -1;
  }


  fvmCommHandle_t commhandle = (fvmCommHandle_t) getAck();

  return commhandle;
}

fvmCommHandle_t fvmGetGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle)
{

  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, scratchHandle, GETGLOBAL);

#ifndef NDEBUGCOMM
  printf("PC: GetGlobal received handle %d\n",commhandle);
#endif

  return commhandle;
}

fvmCommHandle_t fvmPutGlobalData(const fvmAllocHandle_t handle,
				 const fvmOffset_t fvmOffset,
				 const fvmSize_t size,
				 const fvmShmemOffset_t shmemOffset,
				 const fvmAllocHandle_t scratchHandle)
{

  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, scratchHandle, PUTGLOBAL);

#ifndef NDEBUGCOMM
  printf("PC: PutGlobal received handle %d\n",commhandle);
#endif

  return commhandle;

}

fvmCommHandle_t fvmPutLocalData(const fvmAllocHandle_t handle,
				const fvmOffset_t fvmOffset,
				const fvmSize_t size,
				const fvmShmemOffset_t shmemOffset)
{

  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, 0, PUTLOCAL);

#ifndef NDEBUGCOMM
  printf("PC: PutLocal received handle %d\n",commhandle);
#endif

  return commhandle;
}


fvmCommHandle_t fvmGetLocalData(const fvmAllocHandle_t handle,
				const fvmOffset_t fvmOffset,
				const fvmSize_t size,
				const fvmShmemOffset_t shmemOffset)
{

  fvmCommHandle_t commhandle = fvmCommData(handle, fvmOffset, size, shmemOffset, 0, GETLOCAL);

#ifndef NDEBUGCOMM
  printf("PC: GetLocal received handle %d\n",commhandle);
#endif

  return commhandle;

}

// wait on communication between fvm and pc
fvmCommHandleState_t waitComm(fvmCommHandle_t handle)
{
//   fvmRequest_t request = {
//     WAITCOMM
//     , { 0
//         , 0
//         , 0
//         , handle
//         , 0
//         , 0
//     }
//   };
  fvmRequest_t request;
  request.op = WAITCOMM;
  request.args.arg_commhandle  = handle;

  if (handle < 0)
    {
      throw std::runtime_error ("waitComm: invalid handle");
    }

  if(doRequest(request))
    perror("error doing request");


  return (fvmCommHandleState_t) getAck();

}

void *fvmGetShmemPtr()
{
#ifdef SHMEM

#ifndef NDEBUG
  printf("PC: return shmem pointer is %p\n", pcShm);
#endif

  return pcShm;
#else
  return 0;
#endif
}

fvmSize_t fvmGetShmemSize()
{
#ifdef SHMEM
  return pcShmSize;
#endif
  return 0;
}

int fvmGetRank()
{
  return nodeRank;
}
int fvmGetNodeCount()
{
  return nodeCount;
}

static void selftest (void *, const we::loader::input_t &, we::loader::output_t & out)
{
  std::cerr << "running self test" << std::endl;
  out.bind ("result", 0L);
}

WE_MOD_INITIALIZE_START (fvm);
{
  fvmSize_t shmem_size (0);
  fvmSize_t fvm_size (0);
  if (getenv("FVM_PC_SHMSZ"))
  {
	shmem_size = (fvmSize_t)atoll(getenv("FVM_PC_SHMSZ"));
  }
  if (getenv("FVM_PC_FVMSZ"))
  {
	fvm_size = (fvmSize_t)atoll(getenv("FVM_PC_FVMSZ"));
  }
  std::cerr << "fvm-pc: init: FVM_PC_SHMSZ=" << shmem_size << std::endl;
  std::cerr << "fvm-pc: init: FVM_PC_FVMSZ=" << fvm_size << std::endl;
  fvm_pc_config_t cfg (getenv("FVM_PC_MSQ"), getenv("FVM_PC_SHM"), shmem_size, fvm_size);
  int res(0);
  if ( (res = fvmConnect (cfg)) != 0)
  {
    throw std::runtime_error("could not initialize fvm-connection: "+fhg::util::show(res));
  }

  WE_REGISTER_FUN (selftest);
}
WE_MOD_INITIALIZE_END (fvm);

WE_MOD_FINALIZE_START (fvm);
{
  fvmLeave ();
}
WE_MOD_FINALIZE_END (fvm);
