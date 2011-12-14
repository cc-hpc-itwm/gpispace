#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>

#include <mmgr/dtmmgr.h>
#include <mmgr/tmmgr.h>
#include <Pv4dVM4.h>

#include <fvmAllocator.h>
#include <fvmAllocatorTypes.h>
#include <fvmSync.h>
#include <fvmLogger.h>


#define MAXNODES 1024

/* internal state */
static void *fvmMemStartAddr;
static unsigned long fvmMem;
static unsigned long fvmMemUsed;
static unsigned long fvmMemFree;
static unsigned long numAllocations=0;
/* sockets to other allocator threads */
static int sockfd[MAXNODES];

pthread_t allocator_thread;

static unsigned int global_alloc_in_progress = 0;
static unsigned int local_alloc_in_progress = 0;
static pthread_mutex_t mutex_alloc_in_progress;
static pthread_mutex_t mutex_localAlloc;
static pthread_cond_t cond_alloc_in_progress;

static fvmCondVar_t fvmcond_global_alloc;
static fvmMutex_t fvmmutex_global_alloc;


DTmmgr_t dtmmgr = (DTmmgr_t) NULL;

int myrank=-1;
static int numnodes=-1;
static const char **hostnames;

static int localsockpair[2];


/*****************************  Utilities **********************************************/

const char *type2str(fvmAllocType_t type)
{

  static const char * const fvm_type_str[] = {
    /*
    [QUERYGLOBALLOC] = "Query global allocation",
    [QUERYGLOBALLOCACK] = "Query global allocation ack",
    [ABORTGLOBALLOC] = "Abort global allocation",
    [ABORTGLOBALLOCACK] = "Abort global allocation ack",
    [COMMITGLOBALLOC] = "Commit global allocation",
    [COMMITGLOBALLOCACK] = "Commit global allocation ack",
    [COMMSTOPGLOBAL] = "Communication stop global",
    [COMMSTOPGLOBALACK] = "Communication stop global ack",
    [COMMITGLOBALLOCACKGLOBALLY] = "Commit global allocation globally",
    [LOCALLOC] = "Local allocation",
    [LOCALFREE] = " Local deallocation",
    [GLOBALFREE] = "Global deallocation",
    [GLOBALFREEACK] = "global deallocation ack"
    */
    "Query global allocation",
    "Query global allocation ack",
    "Abort global allocation",
    "Abort global allocation ack",
    "Commit global allocation",
    "Commit global allocation ack",
    "Communication stop global",
    "Communication stop global ack",
    "Commit global allocation globally",
    "Local allocation",
    " Local deallocation",
    "Global deallocation",
    "global deallocation ack"
  };

  if(type < QUERYGLOBALLOC || type > GLOBALFREEACK)
    return "unknown";

  return fvm_type_str[type];
}

static void printAllocRequest(fvmAllocRequest_t request)
{
  fvm_printf("Request type: %s \n size: %lu\n root: %d\n handle: %lu\n",
	     (char *) type2str(request.type),
	     (unsigned long)request.size,
	     (int)request.root,
	     (unsigned long)(request.handle));
}


/************************ Internal functions to allocator ***************************/

/* Handle */
/*____________ _______________________ */
/*|  rank    ||       numAllocations | */
/*| 20 bits  ||            44 bits   | */
/*------------------------------------ */

static fvmAllocHandle_t buildHandle()
{
  unsigned long myrankUL = myrank;

  numAllocations++;
  return (fvmAllocHandle_t) ((myrankUL << 44) | (numAllocations & 0xFFFFFFFFFFFUL));
  // return (fvmAllocHandle_t) ((myrankUL << 24) | (numAllocations & 0xFFFFFFUL));
}


/* The broadcast is not a real broadcast in the sense of a collective operation. */
/* Is more like a propagate. It gets a request and sends it to the children. */

static int broadcast(fvmAllocRequest_t *request)
{

  int dst, ret=0;
  int mask=0x1;
  int relative_rank = (myrank >= request->root) ? myrank - request->root: myrank - request->root +numnodes;

  int bytes;


#ifndef NDEBUGALLOCCOMM
  static int numBcast;
  numBcast++;
  fvm_printf("Enter Broadcast %d\n", numBcast);
#endif

  /* taken from MPI, mvapich - modified to do simply a propagate, not a real broadcast */
  while(mask < numnodes){
    if(relative_rank & mask)
      break;
    mask <<=1;
  }
  mask >>=1;


  while (mask > 0){
    if(relative_rank + mask < numnodes){
      dst = myrank + mask;
      if(dst >= numnodes) dst -= numnodes;

#ifndef NDEBUGALLOCCOMM
      fvm_printf("Broadcast %s to %d\n",type2str(request->type),dst);
#endif

      bytes= send(sockfd[dst],request,sizeof(fvmAllocRequest_t),0);
      if(bytes == -1){
	fvm_printf("Thread allocator: couldnt send broadcast request to allocator %d\n",dst);
	ret = 1;
      }
      if(bytes < (sizeof(fvmAllocRequest_t)))
	fvm_printf("Thread allocator: couldnt send broadcast _complete_ request to allocator %d\n",dst);
    }
    mask >>=1;

  }

#ifndef NDEBUGALLOCCOMM
  fvm_printf("Broadcast done\n");
#endif

  return ret;
}



static int reduce(fvmAllocRequest_t *request)
{

  int dst, src,ret = 0;
  int mask=0x1;
  int relative_rank = (myrank >= request->root) ? myrank - request->root: myrank - request->root +numnodes;
  fvmAllocRequest_t recvReq;

#ifndef NDEBUGALLOCCOMM
  static int numReduce;
  numReduce++;
  fvm_printf("Reduce %d request %s\n", numReduce, type2str(request->type));
#endif

  while(mask < numnodes){
    if(relative_rank & mask)
      break;
    mask <<=1;
  }

  mask >>= 1;

  while (mask > 0)
    {
      if(relative_rank + mask < numnodes)
  	{
  	  dst = myrank + mask;
  	  if(dst >= numnodes) dst -= numnodes;
#ifndef NDEBUGALLOCCOM
	  fvm_printf("Receiving from %d\n",dst);
#endif
  	  if((recv(sockfd[dst],&recvReq,sizeof(fvmAllocRequest_t),0)) == -1){
  	    fvm_printf("Thread allocator: couldnt send reduce request to allocator %d\n",dst);
  	    ret = 1;
  	  }
	  if(ret || recvReq.handle == 0)
            request->handle = 0;
  	}
      mask >>=1;

    }


  mask = 0x1;

  while (mask < numnodes){
    if(relative_rank & mask){
      src = myrank - mask;
      if(src < 0) src +=numnodes;
#ifndef NDEBUGALLOCCOM
      fvm_printf("Sending to %d\n",src);
#endif
      if((send(sockfd[src],request,sizeof(fvmAllocRequest_t),0)) == -1){
	fvm_printf("Thread allocator: couldnt send request to allocator %d\n",src);
	ret = 1;
      }
      break;
    }
    mask <<=1;
  }

  return ret;

}

static HandleReturn_t fvmMMFreeInternal(fvmAllocHandle_t handle, Arena_t arena)
{

  MemSize_t size = 0;

  dtmmgr_offset_size (dtmmgr, handle, arena, NULL, &size);

  HandleReturn_t ret = dtmmgr_free(&dtmmgr, (Handle_t)handle, arena);

  if(ret == RET_SUCCESS){

    if((size_t)(size) > 0)
      {

	fvmMemUsed -= (size_t)(size);
	fvmMemFree += (size_t)(size);
      }
  }
#ifndef NDEBUGALLOC
  else
    {
      fvm_printf("FVM ALLOCATOR: free failed for handle %lu on arena %d \n", handle, arena);
    }
#endif

#ifndef NDEBUGALLOC
  dtmmgr_status (dtmmgr);
  fvm_printf("FVM ALLOCATOR: free returns %d \n", ret);
#endif

  return ret;
}

static void fMemmove (const OffsetDest_t OffsetDest, const OffsetSrc_t OffsetSrc,
		      const MemSize_t Size, void *PDat)
{
/* #ifndef NDEBUGALLOC */
/*   printf ("CALLBACK-%lu: Moving " FMT_MemSize_t " Byte(s) from " FMT_Offset_t */
/* 	  " to " FMT_Offset_t "\n", (*(unsigned long *) PDat)++, Size, */
/* 	  OffsetSrc, OffsetDest); */
/* #endif */

  void * ptr = memmove( (char *) fvmMemStartAddr + OffsetDest, (char *)fvmMemStartAddr + OffsetSrc, Size);

}

/* allocation itself */
/* with the dtmmgr we can have this in a single function for both local and global */
static AllocReturn_t fvmMMAllocInternal(size_t size, fvmAllocHandle_t handle, Arena_t arena)
{
#ifndef NDEBUGALLOC
  fvm_printf("Allocation status:\n");
#endif

  AllocReturn_t allocReturn = ALLOC_FAILURE;

  if(dtmmgr == NULL)
    return allocReturn;

  allocReturn = dtmmgr_alloc(&dtmmgr,(Handle_t)handle, arena, size);

  /* if necessary defragment */
  if(allocReturn == ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY)
    {
#ifndef NDEBUGALLOC
      fvm_printf("FVM ALLOCATOR: Need to do defragmentation\n");
#endif

      dtmmgr_defrag (&dtmmgr, arena ,&fMemmove, NULL, NULL);
      allocReturn = dtmmgr_alloc(&dtmmgr, (Handle_t)handle, arena, size);
    }

  if(allocReturn == ALLOC_SUCCESS)
    {
      fvmMemUsed += size;
      fvmMemFree -= size;
    }

#ifndef NDEBUGALLOC
  dtmmgr_status (dtmmgr);
#endif

  return allocReturn;

}



/**************************** Interface for allocations **********************************/

int fvmGlobalMMFree(fvmAllocHandle_t handle)
{

  fvmAllocRequest_t request;
  request.type = GLOBALFREE;
  request.handle = handle;
  int ack = 0;

  unsigned long currentAllocator ;
  currentAllocator  = atomicCmpSwapCntVM(0, myrank+1, VMCounter7);
  fvm_printf("FVM ALLOCATOR: current allocator at start is %lu\n", currentAllocator);

  //  do {} while (atomicCmpSwapCntVM(0, myrank+1, VMCounter7) != myrank+1);
  if(currentAllocator  != 0)
    {
      do
	{
	  if(fvmCondWait(&fvmcond_global_alloc, &fvmmutex_global_alloc, hostnames[currentAllocator - 1]) < 0)
	    {
	      ack = 1;
	      goto out;
	    }
	  //	  sleep(1);
	  /* try to update current allocator to be me */
	  currentAllocator = atomicCmpSwapCntVM(0, myrank+1, VMCounter7);
	  fvm_printf("FVM ALLOCATOR: current allocator after signal %lu\n", currentAllocator);

	} while(currentAllocator != 0);
    }


  /* request to thread allocator */
  if((send(localsockpair[0],&request,sizeof(request),0)) == -1){
    fvm_printf("Could not send to thread allocator\n");
    return -1;
  }

  if((recv(localsockpair[0],&ack,sizeof(ack),0)) == -1)
    fvm_printf("Could not ack from thread allocator\n");

  currentAllocator = atomicCmpSwapCntVM(myrank + 1, 0, VMCounter7);

  fvmCondSignal(&fvmcond_global_alloc);
 out:
  return ack;

}


static int sendMsg2GlobalAllocator(fvmAllocRequest_t *request, fvmAllocType_t  type)
{
  request->type = type;

  if((send(localsockpair[0],request,sizeof(fvmAllocRequest_t),0)) == -1)
    {
#ifndef NDEBUG
      fvm_printf("Could not send to thread allocator\n");
#endif

      return -1;
    }

  return 0;
}

static int sendRequestMsg2GlobalAllocator(fvmAllocRequest_t *request, size_t size)
{

  request->type = QUERYGLOBALLOC;
  request->size = size;

  if((send(localsockpair[0], request, sizeof(fvmAllocRequest_t), 0)) == -1)
    {
#ifndef NDEBUG
      fvm_printf("Could not send to thread allocator\n");
#endif

      return -1;
    }

  return 0;
}

static int recvMsg2GlobalAllocator(fvmAllocRequest_t *request)
{
  if((recv(localsockpair[0],request,sizeof(fvmAllocRequest_t),0)) == -1)
    {

#ifndef NDEBUG
      fvm_printf("Could not recv from thread allocator\n");
#endif

      return -1;
    }

  return 0;
}


fvmAllocHandle_t fvmGlobalMMAlloc(fvmSize_t size)
{

  fvmAllocRequest_t request;
  unsigned long currentAllocator ;
  currentAllocator  = atomicCmpSwapCntVM(0, myrank+1, VMCounter7);
  fvm_printf("FVM ALLOCATOR: current allocator at start is %lu\n", currentAllocator);

  //  do {} while (atomicCmpSwapCntVM(0, myrank+1, VMCounter7) != myrank+1);
  if(currentAllocator  != 0)
    {
      do
	{
	  if(fvmCondWait(&fvmcond_global_alloc, &fvmmutex_global_alloc, hostnames[currentAllocator - 1]) < 0)
	    {
	      request.handle = 0;
	      goto out;
	    }
	  //	  sleep(1);
	  /* try to update current allocator to be me */
	  currentAllocator = atomicCmpSwapCntVM(0, myrank+1, VMCounter7);
	  fvm_printf("FVM ALLOCATOR: current allocator after signal %lu\n", currentAllocator);

	} while(currentAllocator != 0);
    }
 fvm_printf("FVM ALLOCATOR: Proceding with allocation rank: %d \n", myrank);

  /* request to thread allocator */
  if ((sendRequestMsg2GlobalAllocator(&request, size)) == -1)
    {
      request.handle = 0;
      goto out;
    }

  if((recvMsg2GlobalAllocator(&request)) == -1)
    {
      request.handle = 0;
      goto out;
    }
  else
    {
      /* if handle is 0 is bad, we have to abort */
      if(request.handle == 0 && request.type == QUERYGLOBALLOCACK){

	if((sendMsg2GlobalAllocator(&request, ABORTGLOBALLOC)) == -1)
	  {
	    request.handle = 0;
	    goto out;
	  }

	else{
	  /* wait for ack */
	  if((recvMsg2GlobalAllocator(&request)) == -1)
	    {
	      request.handle = 0;
	      goto out;
	    }
	}
      }

      /* else proceed with allocation */

      else if(request.handle > 0 && request.type == QUERYGLOBALLOCACK){

	/* initiate commit */
	if((sendMsg2GlobalAllocator(&request, COMMITGLOBALLOC)) == -1)
	  {
	    request.handle = 0;
	    goto out;
	  }
	else
	  {
	    if((recvMsg2GlobalAllocator(&request)) == -1)
	      {
		request.handle = 0;
		goto out;
	      }
	  }

	if((sendMsg2GlobalAllocator(&request, COMMSTOPGLOBAL)) == -1)
	  {
	    request.handle = 0;
	    goto out;
	  }
	else
	  {

	    if((recvMsg2GlobalAllocator(&request)) == -1)
	      {
		request.handle = 0;
		goto out;
	      }
	  }

	/* tell everyone all is fine */
	if((sendMsg2GlobalAllocator(&request, COMMITGLOBALLOCACKGLOBALLY)) == -1)
	  {
	    request.handle = 0;
	    goto out;
	  }
	else
	  {
	    if((recvMsg2GlobalAllocator(&request)) == -1)
	      {
		request.handle = 0;
		goto out;
	      }
	  }
      }

#ifndef NDEBUGALLOC
      fvm_printf("Allocation: received handle  %ld\n",request.handle);
#endif


      /*release global allocation */
      currentAllocator = atomicCmpSwapCntVM(myrank + 1, 0, VMCounter7);
/*       fvm_printf("FVM ALLOCATOR: current allocator is %lu\n", currentAllocator); */

/*       unsigned long c = atomicFetchAddCntVM(0, VMCounter7); */
/*       fvm_printf("FVM ALLOCATOR: current allocator after is %lu\n", c); */

      fvmCondSignal(&fvmcond_global_alloc);

    out:

      return request.handle;
    }
}

fvmAllocHandle_t fvmLocalMMAlloc(fvmSize_t size)
{

  fvmAllocHandle_t handle = 0;

  /* only one local alloc a time */
  pthread_mutex_lock(&mutex_localAlloc);

  if(size <= fvmMemFree && size > 0){

    pthread_mutex_lock(&mutex_alloc_in_progress);

    /* wait for any on going global allocations */
    while(global_alloc_in_progress > 0)
      pthread_cond_wait(&cond_alloc_in_progress,&mutex_alloc_in_progress);

    ++local_alloc_in_progress;
    pthread_mutex_unlock(&mutex_alloc_in_progress);

    handle = buildHandle();

    if(fvmMMAllocInternal(size, handle, ARENA_DOWN) != ALLOC_SUCCESS){
      fvm_printf("Allocator thread: failed to do local allocation\n");
      handle = 0;
    }

    pthread_mutex_lock(&mutex_alloc_in_progress);
    --local_alloc_in_progress;
    pthread_cond_signal(&cond_alloc_in_progress);
    pthread_mutex_unlock(&mutex_alloc_in_progress);

  }
#ifndef NDEBUGALLOC
  else
    {
      fvm_printf("FVM ALLOCATOR: no free mem (%lu) or size (%lu) invalid \n", fvmMemFree, size);
    }
#endif

  pthread_mutex_unlock(&mutex_localAlloc);

#ifndef NDEBUGALLOC
  fvm_printf("FVM ALLOCATOR: received handle  %lu\n",handle);
#endif


  return handle;

}



int fvmLocalMMFree(fvmAllocHandle_t handle)
{
  int ret=0;

  /* stop communication before do the free */
  deferCommunication();
  waitAllCommunication();

  /* make sure there's no alloc/free before the free */
  pthread_mutex_lock(&mutex_localAlloc);
  pthread_mutex_lock(&mutex_alloc_in_progress);

  /* wait for any on going global allocations */
  while(global_alloc_in_progress > 0)
    pthread_cond_wait(&cond_alloc_in_progress,&mutex_alloc_in_progress);

  ++local_alloc_in_progress;
  pthread_mutex_unlock(&mutex_alloc_in_progress);

  if(fvmMMFreeInternal(handle, ARENA_DOWN) != RET_SUCCESS) {
    ret = 1;
  }

  pthread_mutex_lock(&mutex_alloc_in_progress);
  --local_alloc_in_progress;
  pthread_cond_signal(&cond_alloc_in_progress);
  pthread_mutex_unlock(&mutex_alloc_in_progress);

  pthread_mutex_unlock(&mutex_localAlloc);

  allowCommunication();

  return ret;
}



/*  TODO: maybe add via pthread_cleanup_handler */
static void allocator_thread_exit(void *arg)
{
  /* clean all resources */
  Size_t bytes = dtmmgr_finalize (&dtmmgr);

  pthread_mutex_destroy(&mutex_alloc_in_progress);
  pthread_mutex_destroy(& mutex_localAlloc);
  pthread_cond_destroy(&cond_alloc_in_progress);

  fvmCondDestroy(&fvmcond_global_alloc);
  fvmMutexDestroy(&fvmmutex_global_alloc);

}

/* Allocator thread for global allocations */
/* The allocator is a thread waiting for requests to do a global allocation. */
void *allocator_thread_f(void * args)
{
  int i,j;

  fd_set master;    /* master file descriptor list */
  fd_set read_fds;  /* temp file descriptor list for select() */
  int fdmax;        /* maximum file descriptor number */

  int yes=1;

  int listensock;

  struct addrinfo hints;
  struct addrinfo *servinfo, *p;

  int nbytes;

  dtmmgr_init (&dtmmgr, fvmMem, 1); /* TODO: alignment? */

#ifndef NDEBUGALLOC
  dtmmgr_info (dtmmgr);
#endif

  pthread_mutex_init(&mutex_alloc_in_progress,NULL);
  pthread_mutex_init(&mutex_localAlloc,NULL);
  pthread_cond_init(&cond_alloc_in_progress,NULL);

  pthread_mutex_init(&mutex_communication,NULL);
  pthread_cond_init(&cond_communication,NULL);

 /*  pthread_cleanup_push(allocator_thread_exit, NULL); */


  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  FD_ZERO(&master);
  FD_ZERO(&read_fds);


  for(i=0;i<MAXNODES;i++)
    sockfd[i]=-1;


  fdmax = localsockpair[1];

  FD_SET(localsockpair[1],&master); /* add socketpair to own node */


  /*************************  start listening *********************************************/

  if ((getaddrinfo(NULL, ALLOCATOR_PORT_REMOTE, &hints, &servinfo)) != 0)
    {
      pthread_exit(NULL);
    }


  /* loop through all the results and bind to the first we can */
  for(p = servinfo; p != NULL; p = p->ai_next)
    {
      if ((listensock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
	{
	  continue;
	}

      if (setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
	  pthread_exit(NULL);
	}

      if (bind(listensock, p->ai_addr, p->ai_addrlen) == -1)
	{
	  close(listensock);
	  continue;
	}
      break;
    }

  if (p == NULL)
    {
      pthread_exit(NULL);
    }

  freeaddrinfo(servinfo);

  if (listen(listensock, BACKLOG) == -1)
    {
      pthread_exit(NULL);
    }

  /************************************ now build topology ************************************/

#ifndef NDEBUG
  fvm_printf("************ Allocator thread: building topology *************\n");
#endif


  /* TODO: why not 2 loops to connect nodes
     for (other = myrank + 1; other < numnode; ++other)
     connect(other);

     for (other = 0; other < myrank; ++other)
     accept(other);

     instead of the nested loop with the if
  */

  for(i=0; i<numnodes; i++)
    {
      /* if it's me, then connect to all nodes after me */
      if(myrank == i)
	{

	  for(j=i+1;j<numnodes;j++)
	    {

	      if ((getaddrinfo(hostnames[j], ALLOCATOR_PORT_REMOTE, &hints, &servinfo)) != 0)
		{
		  pthread_exit(NULL);
		}
	      /* loop through all the results and connect to the first we can */
	      for(p = servinfo; p != NULL; p = p->ai_next)
		{
		  if ((sockfd[j] = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1)
		    {
		      perror("client: socket");
		      continue;
		    }

		  if (connect(sockfd[j], p->ai_addr, p->ai_addrlen) == -1)
		    {
		      close(sockfd[j]);
		      perror("thread allocator: connect");
		      continue;
		    }
#ifndef NDEBUG
		  fvm_printf("Thread allocator: connected to node %d\n",j);
#endif
		  break;
		}

	      if (p == NULL)
		{
		  fvm_printf("Thread allocator: error, no usable sockets");
		  pthread_exit(NULL);

		}
	      FD_SET(sockfd[j],&master);
	      if (sockfd[j] > fdmax)
		{
		  fdmax = sockfd[j];
		}
	    }
	}

      /* accept connection from nodes before me */
      if(myrank > i)
	{

	  int lfd = -1;

	  struct sockaddr_in Sender;
	  socklen_t SenderSize = sizeof(Sender);

	  if(listensock==-1){
	    pthread_exit(NULL);
	  }

	  lfd = accept(listensock,(struct sockaddr*)&Sender,&SenderSize);
	  if(lfd == -1)
	    pthread_exit(NULL);

	  const char *clientIP = inet_ntoa(Sender.sin_addr);

	  struct in_addr addr;
	  addr.s_addr = inet_addr(clientIP);
	  struct hostent *host;
	  host = gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);

	  if(!host)
	    {
	      pthread_exit(NULL);
	    }

	  const char *hnName = host->h_name;

	  int pos=-1;
	  int k;
	  for(k=0; k<numnodes; k++)
	    {
	      if( strstr(hnName,hostnames[k]) != NULL )
		{
		  pos=k;
		}
	    }

	  /* TODO: O(n^2), how safe is strstr? */

	  if(pos==-1)
	    {
	      pthread_exit(NULL);
	    }

	  sockfd[pos] = lfd;
	  FD_SET(sockfd[pos],&master);
	  if (sockfd[pos] > fdmax)
	    {
	      fdmax = sockfd[pos];
	    }
#ifndef NDEBUG
	  fvm_printf("Thread allocator: connected to node %d\n",pos);
#endif
	}
    }
  /****************************** Topology done ************************************ */
#ifndef NDEBUG
  fvm_printf("************ Allocator thread: building topology *************\n");
#endif

  /* now just wait for requests */
  for(;;)
    {
      read_fds= master;

#ifndef NDEBUGALLOC
      fvm_printf("Alloc thread: waiting in select\n");
#endif

      if(select(fdmax+1, &read_fds,NULL,NULL,NULL) == -1)
	{
	  pthread_exit(NULL);
	}

      for(i=0;i<=fdmax;i++)
	{
	  if(FD_ISSET(i,&read_fds)){

	    fvmAllocRequest_t request;
	    int ack = 0;
	    int local = 0;

	    if(i == localsockpair[1]) /* local request */
	      local = 1;

	    if((nbytes = recv(i,&request,sizeof(fvmAllocRequest_t),MSG_WAITALL)) <=0)
	      {
		if(nbytes == 0)
		  /* connection closed */
		  fvm_printf("allocator thread: socket %d closed\n",i);
		else
		  perror("recv");
		close(i);
		FD_CLR(i,&master);

	      }
	    else
	      {

#ifndef NDEBUGALLOC
		fvm_printf("------- New request %s\n",(char *) type2str(request.type));
#endif

		switch(request.type) {
		case QUERYGLOBALLOC:

		  if(local)
		    {
		      /* I'm initiator of allocation so build handle */
		      request.handle = buildHandle();
		      request.root = myrank;
		    }

#ifndef NDEBUGALLOC
		  printAllocRequest(request);
#endif

		  /* make sure there is no running allocation */
		  pthread_mutex_lock(&mutex_alloc_in_progress);

		  while(local_alloc_in_progress > 0)
		    pthread_cond_wait(&cond_alloc_in_progress,&mutex_alloc_in_progress);

		  ++global_alloc_in_progress;
		  pthread_mutex_unlock(&mutex_alloc_in_progress);

		  /* tell other allocators to alloc with the handle */
		  if(broadcast(&request))
		    fvm_printf("broadcast failed for request type %s\n",(char *) type2str(request.type));


		  if(fvmMemFree < request.size || request.size == 0)
		    {
		      fvm_printf("NOT enough memory or request of size zero\n");
		      request.handle = 0;
		    }

		  /* reduce to find out whether we can commit or abort */
		  request.type = QUERYGLOBALLOCACK;
		  if(reduce(&request))
		    fvm_printf("reduce failed for %s\n",(char *) type2str(request.type));

#ifndef NDEBUGALLOC
		  printAllocRequest(request);
#endif

		  if(local)
		    {
		      /* Im initiator */
		      /* return handle to fvm process */
		      if((send(localsockpair[1],&request,sizeof(fvmAllocRequest_t),0)) == -1)
			fvm_printf("Thread allocator: couldnt handle to local\n");
		    }
		  break;

		case ABORTGLOBALLOC:
		  /* tell other allocators to abort */
		  if(broadcast(&request))
		    fvm_printf("broadcast failed for request type %d\n",(int)request.type);

		  /* actually abort */
		  pthread_mutex_lock(&mutex_alloc_in_progress);
		  --global_alloc_in_progress;
		  pthread_cond_signal(&cond_alloc_in_progress);
		  pthread_mutex_unlock(&mutex_alloc_in_progress);

		  request.type = ABORTGLOBALLOCACK;
		  if(reduce(&request))
		    fvm_printf("reduce failed for %d\n",(int)request.type);

		  if(local)
		    {
		      /* Im initiator */
		      /* return handle to fvm process */
		      if((send(localsockpair[1],&request,sizeof(fvmAllocRequest_t),0)) == -1)
			fvm_printf("Thread allocator: couldnt handle to local\n");
		    }
		  break;

		case COMMITGLOBALLOC:
		  /* tell others to commit allocation */
		  if(broadcast(&request))
		    fvm_printf("broadcast failed for %d\n",(int)request.type);


		  deferCommunication();
		  waitAllCommunication();

		  request.type = COMMITGLOBALLOCACK;
		  if(reduce (&request))
		    fvm_printf("reduce failed for %d\n",(int) request.type);

		  if(local)
		    {
		      if((send(localsockpair[1],&request,sizeof(fvmAllocRequest_t),0)) == -1)
			fvm_printf("Thread allocator: couldnt handle to local\n");
		    }

		  break;

		case COMMSTOPGLOBAL:

		  /* tell others that communication is stopped */
		  if(broadcast(&request))
		    fvm_printf("broadcast failed for %d\n",(int)request.type);


		  /* finally can do the allocation */
		  if(fvmMMAllocInternal(request.size,request.handle,ARENA_UP) != ALLOC_SUCCESS)
		    {
		      fvm_printf("Allocator thread: failed to do allocation\n");
		      request.handle = 0;
		    }

		  request.type = COMMSTOPGLOBALACK;
		  if(reduce(&request))
		    fvm_printf("reduce failed for %d\n",(int)request.type);

		  if(local){
		    /* return handle to fvm process */
		    if((send(localsockpair[1],&request,sizeof(fvmAllocRequest_t),0)) == -1)
		      fvm_printf("Thread allocator: couldnt handle to local\n");
		  }
		  break;

		case COMMITGLOBALLOCACKGLOBALLY:

		  if(broadcast(&request))
		    fvm_printf("broadcast failed for %d\n",(int)request.type);

		  allowCommunication();

		  if(reduce(&request))
		    fvm_printf("reduce failed for %d\n",(int)request.type);

		  pthread_mutex_lock(&mutex_alloc_in_progress);
		  --global_alloc_in_progress;
		  pthread_cond_signal(&cond_alloc_in_progress);
		  pthread_mutex_unlock(&mutex_alloc_in_progress);

		  if(local)
		    {
		      if((send(localsockpair[1],&request,sizeof(fvmAllocRequest_t),0)) == -1)
			fvm_printf("Thread allocator: couldnt handle to local\n");
		    }
		  break;

		  /* TODO: still assuming free is rather problem-less */
		  /* if one node fails, others do their de-allocation without this knowledge */
		case GLOBALFREE:
		  ack = 0;
		  if(local)
		    request.root = myrank;


		  /* tell other allocators to free handle */
		  if(broadcast(&request))
		    fvm_printf("broadcast failed for %d\n",(int)request.type);


		  deferCommunication();
		  waitAllCommunication();

		  if(fvmMMFreeInternal(request.handle, ARENA_UP) != RET_SUCCESS)
		    {
		      ack = 1;
		    }
		  request.type = GLOBALFREEACK;
		  if(reduce(&request))
		    fvm_printf("reduce failed for %d\n",(int)request.type);


		  allowCommunication();

		  if(local)
		    /* ack fvm process */
		    if((send(localsockpair[1],&ack,sizeof(ack),0)) == -1)
		      fvm_printf("Thread allocator: couldnt ackto local\n");

		  break;
		default:
		  break;
		}
	      }
	  }
	}
    }
}


int fvmMMInit(void * ptr, const size_t length, const int rank, const int nnodes, const char **hosts)
{

  int thread_ret;

  myrank = rank;
  numnodes = nnodes;
  hostnames = hosts;

  if(ptr == NULL)
    return -1;

  fvmMemStartAddr = ptr;
  fvmMem = length;
  fvmMemUsed = 0;
  fvmMemFree = length;

  if(fvmCondInit(&fvmcond_global_alloc) < 0)
    return -1;

  if(fvmMutexInit(&fvmmutex_global_alloc) < 0)
    return -1;

  /* setup connection to allocator thread */
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, localsockpair) == -1) {
    perror("socketpair");
    return -1;
  }

  /* start allocator thread and detach it */
  thread_ret = pthread_create(&allocator_thread,NULL,&allocator_thread_f,NULL);
  if(thread_ret != 0)
    return -1;

  if((pthread_detach(allocator_thread)) !=0)
    return -1;


  return 0;
}

/* TODO: finish finalize function */
int fvmMMFinalize()
{

  pthread_cancel(allocator_thread);

  return 0;
}

