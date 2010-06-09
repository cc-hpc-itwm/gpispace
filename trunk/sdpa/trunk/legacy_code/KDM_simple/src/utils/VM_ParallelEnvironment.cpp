/***************************************************************************
                          VM_ParallelEnvironment.cpp  -  description

    Implementation of the ParallelEnvironment class for use with the VirtualMachine.

                             -------------------
    begin                : Tue Jan 09 2007
    copyright            : (C) 2007 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

/**
  *@author Dirk Merten
  */

#include "VM_ParallelEnvironment.h"
void signal_handler(int sig){

        printf("\n\nTerminating ...\n");
        killProcsVM();
        exit(0);
}

const vm_counter ParallelEnvironment::MUTEXCOUNTER;
const vm_counter ParallelEnvironment::GENERALCOUNTER;
const vm_counter ParallelEnvironment::BATCHCOUNTER;
const vm_counter ParallelEnvironment::NODESLOTCOUNTER;
bool ParallelEnvironment::HostlistVMgenerated = false;
int ParallelEnvironment::NumberofAllocatedNodes = 0;

void ParallelEnvironment::StartUp(int argc, 
		  	     char *argv[], 
			     const char *cmdline, 
			     const unsigned long _MemSize, 
			     int& ret,
			     const unsigned int GivenPort,
			     const bool _PerformChecks)
{
    StartSuccess = false;
    NodeActivity = NULL;
    signal(SIGINT,signal_handler);

#ifdef __ALTIVEC__
    QueueDepth = 1000;
#else
    QueueDepth = getQueueDepthVM();
#endif

    MaxComSize = 1024.l*1024.l*1024.l;

    
    if ((cmdline!=NULL)&&(strncmp("/", cmdline, 1) != 0))
    {
	
	ret = -1;
	return;
    }

    MemSize = _MemSize;

    if (MemSize > 16.l*1024.l*1024.l*1024.l)
    {
	
	
    }

    ret = 0;
    
    // performing checks or pings on master process only
    const bool IamMaster = ( isMasterProcVM(argc, argv) == 1);
    const bool PerformChecks = ( IamMaster )?_PerformChecks:false;
    if (!IamMaster) // if i am started as slave, node list has been read by master
	HostlistVMgenerated = true;

    if ( PerformChecks )
    {
	
	const ERROR_TYPE checkerr = CheckEnvironment( (cmdline == NULL)?argv[0]:cmdline);
	if ( checkerr == FATAL_ERROR_GE )
	    ret = -1;
    }
    
    if (ret != 0)
	return;

    ERROR_TYPE err_f = OK_GE;
    // determine the port that is used
    // FindPort executed on masterprocess only. Slave use allways GivenPort
    unsigned short port = ((GivenPort > 1024) || (!IamMaster))?GivenPort:FindPort(err_f);
    if ( err_f != OK_GE )
    {
	
	ret = -1;
	return;
    }

    
    if ( setPortVM(port) != 0)
    {
	ret = -1;
	
	return;
    }
    

    ret = startPv4dVM(argc, argv, cmdline, MemSize);
    if (ret != 0)
    {
	
	return;
    }
    // startPv4dVM() automatically generates the host list
    HostlistVMgenerated = true;
    NumberofAllocatedNodes = getNodeCountVM();
    rank = getRankVM();

    if ( (PerformChecks) )
    {
	const ERROR_TYPE checkerr = CheckRunningEnvironment();
	if ( checkerr == FATAL_ERROR_GE )
	    ret = -1;
    }

    if (ret != 0)
	return;

    MemPtr = getDmaMemPtrVM();
    if (MemPtr == NULL)
    {
	
	ret = -1;
	return;
    }

    // reserve for internal use by the FailsafeBarrier and recover
    free_offset = 0*sizeof(int);

#ifdef __ALTIVEC__
    NumberOfQueues = 8;
#else
    NumberOfQueues = (getNumberOfQueuesVM()<=8)?getNumberOfQueuesVM():8;
#endif
    queue_name[0] = VMQueue0;
    queue_name[1] = VMQueue1;
    queue_name[2] = VMQueue2;
    queue_name[3] = VMQueue3;
    queue_name[4] = VMQueue4;
    queue_name[5] = VMQueue5;
    queue_name[6] = VMQueue6;
    queue_name[7] = VMQueue7;
    queue_reservation[0] = false;
    queue_reservation[1] = false;
    queue_reservation[2] = false;
    queue_reservation[3] = false;
    queue_reservation[4] = false;
    queue_reservation[5] = false;
    queue_reservation[6] = false;
    // Reserve VMQueue7 for fail safe barrier.
    queue_reservation[7] = true;

    ActiveNodeCount = NumberofAllocatedNodes;
    NodeActivity = new bool[NumberofAllocatedNodes];
    ActiveRankList = new int[NumberofAllocatedNodes];
    rankActive = rank;
    for (int inode = 0; inode < NumberofAllocatedNodes; inode++)
    {
	ActiveRankList[inode] = inode;
    }

    SetNodeActivity();
    unsigned long localMemSize = MemSize;
    
    
    atomicResetCntVM(GENERALCOUNTER);
    atomicResetCntVM(MUTEXCOUNTER);
    atomicResetCntVM(BATCHCOUNTER);
    atomicResetCntVM(NODESLOTCOUNTER);

    StartSuccess = true;

    rlimit rlp;
    getrlimit(RLIMIT_DATA, &rlp);
    printf("data seg size  ");
    if (rlp.rlim_cur == RLIM_INFINITY)
	printf("unlimited  ");
    else
	printf("%lu  ", (unsigned long)  rlp.rlim_cur);
    if (rlp.rlim_max == RLIM_INFINITY)
	printf("unlimited  \n");
    else
	printf("%lu  \n", (unsigned long)  rlp.rlim_max);
    getrlimit(RLIMIT_AS, &rlp);
    printf("max memory size  ");
    if (rlp.rlim_cur == RLIM_INFINITY)
	printf("unlimited  ");
    else
	printf("%lu  ", (unsigned long)  rlp.rlim_cur);
    if (rlp.rlim_max == RLIM_INFINITY)
	printf("unlimited  \n");
    else
	printf("%lu  \n", (unsigned long)  rlp.rlim_max);
    getrlimit(RLIMIT_STACK, &rlp);
    printf("stack size  ");
    if (rlp.rlim_cur == RLIM_INFINITY)
	printf("unlimited  ");
    else
	printf("%lu  ", (unsigned long)  rlp.rlim_cur);
    if (rlp.rlim_max == RLIM_INFINITY)
	printf("unlimited  \n");
    else
	printf("%lu  \n", (unsigned long)  rlp.rlim_max);

}

ParallelEnvironment::~ParallelEnvironment()
{
  if (NodeActivity != NULL)
      delete[] NodeActivity;
  if (ActiveRankList != NULL)
      delete[] ActiveRankList;
  if (StartSuccess)
  {
      shutdownPv4dVM();
  }
  else
      shutdownPv4dVM();
}

int ParallelEnvironment::GetRank()
{
  return rankActive;
}

int ParallelEnvironment::GetNodeCount()
{
  return ActiveNodeCount;
}

void ParallelEnvironment::Barrier()
{
    FailSafeBarrier();
}

void ParallelEnvironment::FailSafeBarrier()
{
    unsigned long SignalOffset = MemSize - 2*sizeof(int);
    volatile int* Signal = (volatile int*)&(((char*)MemPtr)[SignalOffset]);
    if ( rank == 0)
    {
	for (int i = 1; i < ActiveNodeCount; i++)
	{			
	    bool commsuccess = true;
	    *Signal = 0;
	    while ( (*Signal != -1) && commsuccess)
	    {
		readMem(SignalOffset, SignalOffset, sizeof(int), i, VMQueue7);
		int ierr = WaitOnQueue(VMQueue7);
		commsuccess = (ierr >= 0);
		usleep(10000);
	    }
	}

	*Signal = 0;
	for (int i = 1; i < ActiveNodeCount; i++)
	{			
	    writeMem(SignalOffset, SignalOffset, sizeof(int), i, VMQueue7);
	    int ierr = WaitOnQueue(VMQueue7);
	    usleep(10000);
	}
    }
    else
    {
	*Signal  = -1;
	while (*Signal != 0 )
	    usleep(10000);

    }
}

void* ParallelEnvironment::getMemPtr()
{
  return MemPtr;
}

int ParallelEnvironment::readMem(const unsigned long localOffset, const unsigned long remOffset, 
				const unsigned int size, const unsigned int nodeID, const vm_queue queue)
{
    int ierr = 0;
    if ( (    (localOffset + size) < MemSize )
	 && ( (remOffset + size) < MemSize ) 
	 && (   nodeID < NumberofAllocatedNodes ) )
    {
	const unsigned int comsteps = size / MaxComSize;

	if (comsteps > 0)
	    
	    
	if ( openDMARequestsVM( queue ) >= QueueDepth - comsteps )
	{
	    
	    ierr = WaitOnQueue( queue );
	    if (ierr != 0)
		
	}


	unsigned int _size = size; 
	unsigned long _localOffset = localOffset; 
	unsigned long _remOffset = remOffset; 
	for (int icomstep=0; icomstep < comsteps; ++icomstep)
	{
	    ierr =  readDmaVM( _localOffset, _remOffset, _size, ActiveRankList[nodeID], queue);
	    if (ierr != 0)
		

	    _size -= MaxComSize;
	    _localOffset += MaxComSize;
	    _remOffset += MaxComSize;
	}
	ierr =  readDmaVM(_localOffset, _remOffset, _size, ActiveRankList[nodeID], queue);
	if (ierr != 0)
	    
    }
    else
    {
	
	ierr = -1;
    }
    return ierr;
}

int ParallelEnvironment::writeMem(const unsigned long localOffset, const unsigned long remOffset,
				const unsigned int size, const unsigned int nodeID, const vm_queue queue)
{
    int ierr = 0;
    if ( (    (localOffset + size) < MemSize )
	 && ( (remOffset + size) < MemSize ) 
	 && (   nodeID < NumberofAllocatedNodes ) )
    {
	const unsigned int comsteps = size / MaxComSize;
	if ( openDMARequestsVM( queue ) >= QueueDepth - comsteps )
	    ierr += WaitOnQueue( queue );

	unsigned int _size = size; 
	unsigned long _localOffset = localOffset; 
	unsigned long _remOffset = remOffset; 
	for (int icomstep=0; icomstep < comsteps; ++icomstep)
	{
	    ierr +=  writeDmaVM( _localOffset, _remOffset, _size, ActiveRankList[nodeID], queue);
	    _size -= MaxComSize;
	    _localOffset += MaxComSize;
	    _remOffset += MaxComSize;
	}
	ierr +=  writeDmaVM(_localOffset, _remOffset, _size, ActiveRankList[nodeID], queue);
    }
    else
    {
	
	ierr = -1;
    }
    return ierr;
}

int ParallelEnvironment::sendMem(const unsigned long localOffset, const unsigned int size, const unsigned int nodeID, const vm_queue queue) {
  int ierr = 0;
  if ( ( (localOffset + size) < MemSize ) && (   nodeID < NumberofAllocatedNodes ) ) {
	ierr = sendDmaVM(localOffset, size, nodeID, queue);
  }
  else {
	
	ierr = -1;
  }
  return ierr;
};

int ParallelEnvironment::recvMem(const unsigned long localOffset, const unsigned int size, const unsigned int nodeID, const vm_queue queue) {
  int ierr = 0;
  if ( ( (localOffset + size) < MemSize ) && (   nodeID < NumberofAllocatedNodes ) ) {
	ierr = recvDmaVM(localOffset, size, nodeID, queue);
  }
  else {
	
	ierr = -1;
  }
  return ierr;
};

int ParallelEnvironment::Send(char* Var, const int size,
			       const unsigned int nodeID)
{
    return sendSocketVM(Var, size, ActiveRankList[nodeID]);
}

int ParallelEnvironment::Receive(char* Var, const int size,
				  const unsigned int nodeID)
{
    return recvSocketVM(Var, size, ActiveRankList[nodeID]);
}


bool ParallelEnvironment::ReserveQueue(vm_queue& qu)
{
    bool found = false;
    for (int iqu = 0; iqu < NumberOfQueues; iqu++)
    {
	if ( !queue_reservation[iqu] )
	{
	    qu = queue_name[iqu];
	    queue_reservation[iqu] = true;
	    found = true;
	    break;
	}
    }
    return found;
}

void ParallelEnvironment::ReleaseQueue(const vm_queue& qu)
{
    for (int iqu = 0; iqu < NumberOfQueues; iqu++)
    {
	if (queue_name[iqu] == qu)
	    queue_reservation[iqu] = false;
    }
}

int ParallelEnvironment::WaitOnQueue(const vm_queue queue)
{
#ifdef __ALTIVEC__
    const int ierr =  waitDmaVM(queue);
#else
    const int ierr =  waitDma2VM(queue);
#endif

    if (ierr < 0)
	
    return ierr;
}

int ParallelEnvironment::Broadcast(char*& Buffer, int& Size, const int SrcRank)
{
    int ierr = 0;
    if (ActiveRankList[SrcRank] == rank)
    {
	for (int irank = 0; irank < ActiveNodeCount; irank++)
	{
	    if (ActiveRankList[irank] != SrcRank)
	    {
		ierr += Send((char*) &Size, sizeof(int), irank);
		ierr += Send(Buffer, Size, irank);
	    }
	}
    }
    else
    {
	ierr += Receive((char*) &Size, sizeof(int), SrcRank);
	Buffer = new char[Size];
	ierr += Receive(Buffer, Size, SrcRank);
    }
    return ierr;
}

unsigned int ParallelEnvironment::CounterAdd(unsigned int val)
{
  return atomicFetchAddCntVM(val, GENERALCOUNTER);
}

int ParallelEnvironment::CounterReset()
{
    return atomicResetCntVM(GENERALCOUNTER);
}

//########################################################
int ParallelEnvironment::InitBatch(const int StartVal)
{
    int ret = 0;
    if ( rank == 0)
    {
	const int ierr = atomicResetCntVM(BATCHCOUNTER);
	const int val = atomicFetchAddCntVM(StartVal + 1, BATCHCOUNTER);
	if ( (ierr != 0) || (val != 0) )
	    ret = -1;
    }
    return ret;
}

//########################################################
int ParallelEnvironment::GetBatch()
{
    const int val = atomicFetchAddCntVM(1, BATCHCOUNTER) - 1;
    if ( val == -1 ) // counter has been -1, i.e. error report
    {
	ReportErrorBatch();
	return -1;
    }
    else
	return val;
}

//########################################################
int ParallelEnvironment::ReportErrorBatch()
{
    const int ierr = atomicResetCntVM(BATCHCOUNTER);
    return ierr;
}

void ParallelEnvironment::MutexLock(const unsigned int delay)
{
     while (atomicCmpSwapCntVM(0, rank+1 , MUTEXCOUNTER ) != rank+1)
 	usleep(delay);
};


void ParallelEnvironment::MutexUnlock()
{
    atomicResetCntVM(MUTEXCOUNTER);
}

void ParallelEnvironment::NodeSlotLock(const unsigned int delay)
{
     while (atomicCmpSwapCntVM(0, rank+1 , NODESLOTCOUNTER ) != rank+1)
 	usleep(delay);
};


void ParallelEnvironment::NodeSlotUnlock()
{
    atomicResetCntVM(NODESLOTCOUNTER);
}

int ParallelEnvironment::NodeSlotReport()
{
    const int val = atomicFetchAddCntVM(0, NODESLOTCOUNTER) - 1;
    return val;
}

void ParallelEnvironment::NodeSlotFinalize()
{
    const int val = atomicFetchAddCntVM(GetNodeCount() + 1, NODESLOTCOUNTER);
    if ( val != 0)
    {
	
	
    }
}


void ParallelEnvironment::ExitOnError(const int err)
{
    int errsum = err;
    if ( StartSuccess)
    {
	if (rank == 0)
	{
	    for (int isrc = 1; isrc < ActiveNodeCount; isrc++)
	    {
		int errtmp = 0;
		Receive((char*) &errtmp, sizeof(int), isrc);
		errsum += errtmp;
		if (errtmp != 0)
		    
	    }
	    for (int idest = 1; idest < ActiveNodeCount; idest++)
		Send((char*) &errsum, sizeof(int), idest);
	}
	else 
	{
	    Send((char*) &err, sizeof(int), 0);	
	    Receive((char*) &errsum, sizeof(int), 0);
	}

	if (errsum != 0)
	    exit(1);
    }
    else
    {
	exit(1);
    }
}

bool ParallelEnvironment::GetNodeActivity(const int nodeID)
{
    return true;
    if ((nodeID < 0) || (nodeID >= NumberofAllocatedNodes))
	return false;
    else
	return NodeActivity[nodeID];
}

bool ParallelEnvironment::Recover()
{
#ifdef __ALTIVEC__
  return true;
#else

    // Check all nodes and update node activity list
    bool totalcommsuccess = true;
    for (int i = 0; i < ActiveNodeCount; i++)
    {			
	if ( ActiveRankList[i] != rank)
	{
	    int ierr_tmp1 = readMem(1 * sizeof(int), 0, sizeof(int), i, VMQueue0);
	    int ierr = waitDma2VM(0);
	    bool commsuccess = (ierr >= 0);

	    if (ierr_tmp1 < 0)
	    {
		printf("readMem failed for node %i\n", i);
		NodeActivity[ActiveRankList[i]] = false;
		totalcommsuccess = false;
	    }
	    else
	    {
		if (!commsuccess)
		{
		    unsigned char * ePtr = getErrorVectorVM(0);
		    for (int j = 0; j < NumberofAllocatedNodes; j++)
		    {
			if (ePtr[j] != 0)
			    NodeActivity[j] = false;
			
		    }
		    sleep(1);
		    readMem(1 * sizeof(int), 0, sizeof(int), i, VMQueue0);
		    int ierr_tmp = waitDma2VM(0);
		    if (ierr_tmp < 0)
		    {
			unsigned char * ePtr = getErrorVectorVM(0);
			for (int j = 0; j < NumberofAllocatedNodes; j++)
			{
			    printf("ErrorVector for node %i is %i\n", j, ePtr[j]);
			}
		    }
		    totalcommsuccess = false;
		}
	    }
	}
    }

    // Reset
    ActiveNodeCount = 0;
    for (int i = 0; i < NumberofAllocatedNodes; i++)
    {
	if (NodeActivity[i])
	{
	    ActiveRankList[ActiveNodeCount] = i;
	    if (i == rank)
		rankActive = ActiveNodeCount;
	    ActiveNodeCount++;
	}
    }
    
    atomicResetCntVM(GENERALCOUNTER);
    atomicResetCntVM(MUTEXCOUNTER);
    atomicResetCntVM(BATCHCOUNTER);

    return totalcommsuccess;
#endif
}

void ParallelEnvironment::SetNodeActivity()
{
    for (int inode = 0; inode < NumberofAllocatedNodes; inode++)
    {
	NodeActivity[inode] = true;
    }
    return;
    for (int inode = 0; inode < NumberofAllocatedNodes; inode++)
    {
	if (inode != rank)
	{
	    int ierr1 = readDmaVM(0, 0, 1, inode, VMQueue0);
	    int ierr2 = waitDmaVM(VMQueue0);
	    NodeActivity[inode] = ( (ierr1 == 0) && (ierr2 == 0) );
	}
	ActiveRankList[inode] = inode;
    }
}

ERROR_TYPE ParallelEnvironment::CheckRunningEnvironment()
{
    ERROR_TYPE err_flg = OK_GE;
#ifdef VM3
    
    
#else
    const int NumberofAllocatedNodes = GetAllocatedNodeCount();
    { // search for running process
	
	for (int irank(0); irank < NumberofAllocatedNodes; irank++)
	{
	    const char * hostname = getHostnameVM( irank);
	    const int findProc_rtv = findProcVM(hostname);
	    if ( findProc_rtv != 0)
	    {
		if ( findProc_rtv == -42 )
		{
		    
		    
		}
		else
		{
		    
		    err_flg = FATAL_ERROR_GE;
		}
 	     }
	}
	if (err_flg == OK_GE)
	    
    }
#endif
    return err_flg;
}

ERROR_TYPE ParallelEnvironment::CheckEnvironment(const char* prgPath)
{
    ERROR_TYPE err_f = OK_GE;
#ifdef VM3
    
    
#else
    const int NumberofAllocatedNodes = GetAllocatedNodeCount();
    { // ping all demons
	
	for (int irank(0); irank < NumberofAllocatedNodes; irank++)
	{
	    const char * hostname = getHostnameVM( irank);
	    if (pingPv4dDaemonVM(hostname) != 0)
	    {
		
		err_f = FATAL_ERROR_GE;
	    }
	}
	if (err_f == OK_GE)
	    
	else
	  return err_f;
    }
    
    { // run infiniband tests
	
	for (int irank(0); irank < NumberofAllocatedNodes; irank++)
	{
	    const char * hostname = getHostnameVM( irank);
	    const int checkIB_rtv = runIBTestVM( hostname);
	    if ( checkIB_rtv != 0)
	    {
		if ( checkIB_rtv == -42 )
		{
		    
		    
		}
		else
		{
		    
		    err_f = FATAL_ERROR_GE;
		}
	    }
	}
	if (err_f == OK_GE)
	    
    }
    
    { // check for shared libraries
	
	for (int irank(0); irank < NumberofAllocatedNodes; irank++)
	{
	    const char * hostname = getHostnameVM( irank);
	    const int check_rtv = checkSharedLibsVM(hostname, prgPath);
	    if ( check_rtv  != 0 )
	    {
		if ( check_rtv == -42 )
		{
		    
		    
		}
		else
		{
		    
		    err_f = FATAL_ERROR_GE;
		}
	    }
	}
	if (err_f == OK_GE)
	    
    }
#endif
    return err_f;
}

unsigned short ParallelEnvironment::FindPort(ERROR_TYPE& err_f )
{
#ifdef VM3
    
    
    unsigned short port = 0;
#else
    unsigned short current_port = getPortVM();
    const unsigned short last_port = current_port + 1000;
    bool port_is_free = false;

    unsigned short port = current_port; 
    const int NumberofAllocatedNodes = GetAllocatedNodeCount();
    
    while ( (port_is_free == false) && ( current_port < last_port) )
    {
	port_is_free = true;
	
	for (int irank(0); irank < NumberofAllocatedNodes; irank++)
	{
	    const char * hostname = getHostnameVM( irank);
	    const int checkport1_rtv = checkPortVM( hostname, current_port);
	    const int checkport2_rtv = checkPortVM( hostname, current_port+1);
	    
	    if ( ( checkport1_rtv != 0) || ( checkport2_rtv != 0) )
	    {
		if ( ( checkport1_rtv == -42) && ( checkport2_rtv == -42) ) // timeout on both ?
		{
		    
		    
		}
		else // or error on one
		    port_is_free = false;
	    }
	}
	port = current_port;
	++current_port;
    }

    err_f =  ( port_is_free )?OK_GE:FATAL_ERROR_GE;
#endif
    return port;
}
