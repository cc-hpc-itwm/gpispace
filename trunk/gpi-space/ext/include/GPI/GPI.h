/*! \file GPI.h
    \brief The GPI interface.
*/

#ifndef GPI_H
#define GPI_H

#ifdef __cplusplus
extern "C" {
#endif

  //! \name Types
  /*@{*/  
  /*! \enum gpi_queue
   * The available queues for communication
   */
  enum gpi_queue   {GPIQueue0, GPIQueue1, GPIQueue2, GPIQueue3, GPIQueue4, GPIQueue5, GPIQueue6, GPIQueue7};
  
  /*! \enum gpi_counter
   * The available global atomic counters
   */
  enum gpi_counter {GPICounter0, GPICounter1, GPICounter2, GPICounter3, GPICounter4, GPICounter5, GPICounter6, GPICounter7};
  
  /*! \typedef GPI_OP
   * The available operations with collectives 
   */
  typedef enum { GPI_MIN=0, GPI_MAX=1, GPI_SUM=2 } GPI_OP; 
  
  /*! \typedef GPI_TYPE
   * The available data types to work with collectives 
   */
  typedef enum { GPI_INT=0, GPI_UINT=1, GPI_FLOAT=2, GPI_DOUBLE=3, GPI_LONG=4, GPI_ULONG=5 } GPI_TYPE; 

  /*! \typedef GPI_NETWORK_TYPE
   * RDMA networks currently supported by gpi
   */
  typedef enum { GPI_IB=0, GPI_ETHERNET=1 } GPI_NETWORK_TYPE; 

  /*@}*/

  //! \name Infos and Capabilities 
  /*@{*/ 
  //! Get the number of available queues for communication
  /*!
    \return an int with the number of available queues
  */

  int getNumberOfQueuesGPI(void);
  
  //!brief Get the available queue depth
  /*!
    \return an int with the queue depth

    The queue depth is the number of requests that can be posted to a queue without calling a waitDmaGPI on it.
  */
  int getQueueDepthGPI(void);


  //! Get the available global atomic counters
  /*!
    \return an int with the number of global atomic counters

    The number of global atomic counters is limited to the available number. 
    They can be referenced as a int value or using the enum gpi_counter.
  */
  int getNumberOfCountersGPI(void);

  //!brief Returns the GPI version.
  /*!
   
    \return a float with the version.
  */
  float getVersionGPI();

  //! Get the port used by GPI in internal communication with the daemon.
  /*!

    \return an unsigned short with the port number.

    Knowing the port number might be useful to track any
    communication problems with the daemon (e.g. in a system that blocks some ports).
    The default value is 10820.
  */
  unsigned short getPortGPI();

  //! Set the RDMA Network used by GPI.
  /*!

    \param typ The network to use
    \return an int where 0 is success and -1 is failure.

    Setting the rdma communication network to either IB or ETHERNET.
    The default network is IB.
  */

  int setNetworkGPI(GPI_NETWORK_TYPE typ);


  //! Set the port used by GPI in internal communication with the daemon.
  /*!

    \param port The port number to use
    \return an int where 0 is success and -1 is failure.

    Setting the port number to a particular value might be needed if
    the default value (10820) is already in use or blocked.
  */
  int setPortGPI(const unsigned short port);


  //!Set the MTU size for transfer.
  /*!
    \param mtu The MTU size to use.

    \return an int where 0 is success and -1 is failure.

    Sets the MTU size for the communication (readDmaGPI, writeDmaGPI,
    sendDmaGPI, recvDmaGPI, etc.). 

    The default value is 1024 but one should use 2048 and above on
    modern cards. This brings a performance boosts on communication.
  */

  int setMtuSizeGPI(const unsigned int mtu);

  //! Set the number of processes to start GPI.
  /*!
    \param np The number of processes.

    Setting the number of processes is equivalent to setting the
    number of nodes where to run the application. By default GPI
    will start the same number of processes as the number of nodes on
    the machine file. If the application should run on less nodes
    than that number, this function should be used.
  */

  void setNpGPI(const unsigned int np);

  //! Get the total number of nodes.
  /*!

    \return Returns the total number of nodes or -1 on call failure.
  */
  int generateHostlistGPI();

  //! Get the hostname of a node
  /*!
    \param rank The node rank to get the hostname.

    \return a char pointer with the hostname.
  */
  const char *getHostnameGPI(const unsigned int rank);
  /*@}*/


  //! \name Environment runtime check
  /*@{*/
  //! Ping the daemon on a certain node
  /*! 
    \param hostname The hostname where to ping the daemon.

    \return an int where 0 is success and -1 is failure.

    Ping the daemon to find out if the daemon is running on that
    particular hostname.
  */

  int pingDaemonGPI(const char *hostname);


  //! Tells if the calling process is the master process
  /*!
    \param argc The argc of the process
    \param argv The argv of the process

    \return an int where 1 is success, 0 is failure and -42 is timeout.
  */

  int isMasterProcGPI(int argc, char *argv[]);

  //! Check if all shared libraries are found.
  /*!

    \param hostname The hostname where to do the check.
    \return an int where 0 means success, -1 is operation failed, -2 means missing shared libs and -42 is timeout.

    The application runs on a clean environment and therefore some
    libraries might not be found as in the master node. This function
    ensures that everything is OK.
  */

  int checkSharedLibsGPI(const char *hostname);

  //! Check if port is in use.
  /*!
    \param hostname The hostname where to do the check.
    \param portNr The port number to check.
    \return an int where 1 is success, 0 is failure and -42 is timeout.


    Check if the given port number is available to use.
  */

  int checkPortGPI(const char *hostname, const unsigned short portNr);

  //! Find if the program started by daemon really up and running.
  /*!
    \param hostname The hostname where to do the check.
    \return an int where 1 is success, 0 is failure and -42 is timeout.
  */

  int findProcGPI(const char *hostname);

  //! Clear the Linux file cache.
  /*!
    \param hostname The hostname where to execute the clear.
    \return an int where 1 is success, 0 is failure and -42 is timeout.

  */
  int clearFileCacheGPI(const char *hostname);

  //! Check if Infiniband is running properly.
  /*!
    \param hostname The hostname where to run the check.
    \return an in where 0 is success, -1 is operation failed, -2 is no active port/link and -42 is timeout
  */
  int runIBTestGPI(const char *hostname);

  /*@}*/


  //! \name Start and stop GPI
  /*@{*/
  //! Start GPI
  /*!
    \param argc Argument count
    \param argv Command line arguments
    \param cmdline The command line to be used to start the binaries
    \param gpiMemSize The memory space allocated for GPI

    \return an int where -1 is operation failed, -42 is timeout and 0 is success. 

    \warning The command line arguments (argc, argv) won't be forward to the worker nodes

  */
  int startGPI(int argc,char *argv[],const char *cmdline,const unsigned long gpiMemSize);

  //! Shutdown GPI.
  /*!
    \warning This only happens on a node that calls this function. It is not a collective request.

    Cleanly, bring all infrastructure down and releasing resources on the node.
  */
  void shutdownGPI();

  //! Kill the all running GPI processes.
  /*!
    \warning This is a function to be called by the master node
    \return an int where 0 is success and -1 is failure.

    Kills the application on all nodes to avoid hanging
    applications. This should be called on error or failure
    situations.
  */
  int killProcsGPI(void);
  /*@}*/


  //! \name Node utilities
  /*@{*/
  //! Get the node rank.
  /*!
    \return an int with the rank value.

    The rank is the identifier of a node and unique to each node.
  */

  int getRankGPI(void);

  //! Get the number of nodes
  /*!

    \return an int with the number of nodes.
  */
  int getNodeCountGPI(void);

  //! Get the memory reserved for GPI on a worker node.
  /*!

    \return unsigned long with the memory size.

    The Worker node memory size might be different than the Master
    node (e.g. when diferent binaries are started).
  */
  unsigned long getMemWorkerGPI(void);

  //! Get the pointer to the GPI global memory available for DMA operations.
  /*!
    \return void pointer to the gpi memory start region
  */
  void *getDmaMemPtrGPI(void);

  /*@}*/

  //! \name Barriers
  /*@{*/

  //! Execute a fast barrier.
  /*!
    \return void.
  */
  void barrierGPI(void);

  /*@}*/

  //! \name Collectives
  /*@{*/
  //! Allreduce collective operation of a vector of elements (up to 255)
  /*! 
    
    \param sendBuf A pointer to the buffer containing the value.
    \param recvBuf A pointer to the buffer where the result should be placed.
    \param elemCnt The number of elements of the vector
    \param op The type of operation to perform (see enum GPI_OP).
    \param type The datatype of the values to perform the operations (see enum GPI_TYPE).
   
    \return an int where -1 is operation failed and 0 is success.

    The sendBuf and recvBuf don't have to be on the GPI global memory
    regions. Heap or stack memory locations can also be used.

  */
  int allReduceGPI(void *sendBuf,void *recvBuf,const unsigned char elemCnt, GPI_OP op, GPI_TYPE type);

  /*@}*/

  //! \name DMA communication
  /*@{*/
  //! Read data from a remote node memory location
  /*! 
  
    \param localOffset the offset on the node where the data to be transfered whould be written
    \param remOffset the offset on the remote node where the data should be read from
    \param size the size of the transfer (in bytes)
    \param rank the node rank whre to do the transfer
    \param gpi_queue the queue to use for the transfer
  
    \return an int where 0 is success and -1 is operation failed

    This operation is one-sided and non-blocking.
    
  */
  int readDmaGPI(const unsigned long localOffset,const unsigned long remOffset,
		  const int size,const unsigned int rank,const unsigned int gpi_queue);


  //! Write data to a remote node memory location
  /*! 
  
    \param localOffset the offset on the local node where the data to be written is located
    \param remOffset the offset on the remote node where the data should written to
    \param size the size of the transfer (in bytes)
    \param rank the node rank from where to write the data
    \param gpi_queue the queue to use for the transfer
  
    \return an int where 0 is success and -1 is operation failed

    This operation is one-sided and non-blocking.
    
  */
  int writeDmaGPI(const unsigned long localOffset,const unsigned long remOffset,
		  const int size,const unsigned int rank,const unsigned int gpi_queue);



  //! Send data to a remote node
  /*! 
  
    \param localOffset the local offset where the data to be sent is located
    \param size the size of the transfer
    \param rank the node rank where to send the data
    \param gpi_queue the queue to use for the transfer
    \warning 									
    \return an int where 0 is success and -1 is operation failed

    This operation is non-blocking. It is a two-sided operation which
    means that should be a matching recvDmaGPI on the remote node. To
    make sure the operation is done (not hanging on the respective
    queue) a waitDmaGPI needs to be called on the queue.
  */

  int sendDmaGPI(const unsigned long localOffset,const int size,
		const unsigned int rank,const unsigned int gpi_queue);

  //! Receive data from a remote node
  /*! 
  
    \param localOffset the local offset where the received data should be placed
    \param size the size of the data to be received
    \param rank the rank from where to receive
    \param gpi_queue the queue to use for the transfer
    \warning the queue number must match the respective sendDmaGPI queue
    \return an int where 0 is success and -1 is operation failed

    This operation is a blocking operation: it will block until the data
    is received therefore do not call waitDmaGPI for it. There should be
    a correspoding sendDmaGPI from the remote node.
  */

  int recvDmaGPI(const unsigned long localOffset,const int size,
		const unsigned int rank,const unsigned int gpi_queue);
    
  //! Wait for all operations on a queue to finish
  /*! 
  
    \param gpi_queue the queue number to wait on
    \warning Operation not fully thread-safe (read below)
    \return  an int with the number of completed queue events or -1 on error

    WaitDmaGPI makes sure that all operations (read, write, send) posted
    to a queue are finished. The operation is not fully thread-safe:
    when a thread enters a waitDmaGPI, it will wait for all operations
    (including other threads' requests). A thread that posted requests
    on a queue and gets a return value from waitDmaGPI of 0 (zero) only
    knows that some other thread might be waiting or waited for all
    requests.
  */
  int waitDmaGPI(const unsigned int gpi_queue);
  int waitDma2GPI(const unsigned int gpi_queue);

  /*@}*/

  //! \name Passive Communication
  /*@{*/
  //! Send data to a remote node using the passive channel
  /*! 
  
    \param localOffset the local offset where to be sent is located 
    \param size the size of the transfer
    \param rank the node rank where send the data
    \return an int where 0 is success and -1 is operation failed

    This operation uses the passive channel and therefore there's no
    queue for it. It is a non-blocking operation and it is two-sided
    which means that should be a matching recvDmaPassiveGPI on the
    remote node. To make sure the operation is done a waitDmaPassiveGPI
    needs to be called.
  */

  int sendDmaPassiveGPI(const unsigned long localOffset,const int size,const unsigned int rank);


  //! Wait for all passive sends to finish
  /*! 
  
    \warning Operation not fully thread-safe (read below)
    \return  an int with the number of completed queue events or -1 on error

    WaitDmaPassiveGPI makes sure that all send passive operations are
    finished. The operation is not fully thread-safe: when a thread
    enters a waitDmaPassiveGPI, it will wait for all operations
    (including other threads' requests). A thread that posted send
    passive requests and gets a return value from waitDmaPassiveGPI of 0
    (zero) only knows that some other thread might be waiting or waited
    for all requests.

  */
  int waitDmaPassiveGPI();

  //! Receive data using the passive channel
  /*! 
  
    \param localOffset the local offset where to put the received data
    \param size the size of the data to be received
    \param senderRank the rank of the node that sent the data or -1 if the sender could not be established
    \warning this is a blocking function. Do not call waitDmaPassiveGPI for it.
    \warning Not thread-safe. 
    \return an int where 0 is success and -1 is operation failed

    receiveDmaPassiveGPI will block and passively wait (no
    busy-looping) for any incoming data (there is no node
    parameter). There is no need to call waitDmaPassiveGPI after this
    call. The function is not thread-safe and it is advised that only
    a single thread has the function to do passive receives.
  */

  int recvDmaPassiveGPI(const unsigned long localOffset, const int size, int *senderRank);

  /*@}*/

  //! \name Commands
  /*@{*/
  //! Get a command from master node
  /*! 

    \warning this operation is for worker nodes only (all nodes with rank > 0)
  
    \return an int with the command number or -1 on error

    A command is simply a number that can be used to trigger different
    actions on the worker node. This is a blocking operation: it will
    block until the master node has sent the respective command.
  */
  int  getCommandGPI(void);

  //! Set (or send) a command to the worker nodes
  /*! 
  
    \param cmd the command number
    \warning this operation is for the master node only  

    \return an int where 0 is success and -1 is operation failed

    A command is simply a number that can be used to trigger different
    actions on the worker node.  This operation will block until all
    worker nodes have done the respective getCommandGPI.
  */
  int  setCommandGPI(const int cmd);//only master,(rank 0)


  //! Get a command from a particular node
  /*! 
  
    \param rank the rank number to get the command from
    \warning no localhost support
    \return a long with the command number

    A command is simply a number that can be used to trigger different
    actions on the node. This is a blocking operation: it will block
    until the remote node (cannot be itself) has sent the respective
    command.

  */
  long getCommandFromNodeIdGPI(const unsigned int rank);

  //! Set (or send) a command to a particular node
  /*! 
  
    \param rank the node rank where to send the command
    \param cmd the command number to send
    \warning no localhost support  

    \return a long where 0 is success and -1 is operation failed

    This operation will block until the node (rank) has done the
    respective getCommandFromNodeIdGPI.
  */
  long setCommandToNodeIdGPI(const unsigned int rank,const long cmd);

  /*@}*/

  //! \name Tile Counter
  /*@{*/                     
  //! Atomic fetch-and-add of the tile counter
  /*! 
  
    \param val the value to add
  
    \return an unsigned long with the previous (old) value of the tile counter

    The atomic fetch-and-add operation will atomically add the val
    argument to the current value of the counter. The old value is
    returned (fetched).
  */
	
  unsigned long atomicFetchAddTileCntGPI(const unsigned long val);
  
  
  //! Atomic compare-and-swap of the tile counter
  /*! 
    
    \param cmpVal the compare value
    \param swapVal the value to be swapped
    
    \return an unsigned long with the previous (old) value of the tile counter

    The atomic compare-and-swap operation will atomically compare the
    counter value with the argument cmpVal and in case they are equal
    the counter value will be replaced with the swapVal argument. The
    previous (old) value of the counter is returned.

  */
  unsigned long atomicCmpSwapTileCntGPI(const unsigned long cmpVal,const unsigned long swapVal);

  //! Reset the tile counter (make it zero)
  /*! 

    \return an int where 0 is success and -1 is operation failed
  */
  int atomicResetTileCntGPI(void);

  /*@}*/


  //! \name Atomic Counters
  /*@{*/
  //! Atomic fetch-and-add a global atomic counter
  /*! 
    
    \param val the value to be added
    \param gpi_counter the counter number where to do the operation
    \warning avoid busy-looping (e.g. while(1) ) on a counter to wait for a particular value

    \return and unsigned long with the previous (old) value of the global atomic counter

    A global atomic counter is a counter (value) that is globally
    accessible (from all nodes). The operations allowed are
    atomic. The atomic fetch-and-add operation will atomically add the
    val argument to the current value of the counter. The old value is
    returned (fetched).

  */
  unsigned long atomicFetchAddCntGPI(const unsigned long val,const unsigned int gpi_counter);


  //! Atomic compare-and-swap a global atomic counter
  /*! 
    
    \param cmpVal the compare value
    \param swapVal the value to be swapped
    \param gpi_counter the counter number where to do the operation
    \warning avoid busy-looping (e.g. while(1) ) on a counter to wait for a particular value

    \return an unsigned long with the previous (old) value of the atomic counter

    The atomic compare-and-swap operation will atomically compare the
    counter value with the argument cmpVal and in case they are equal
    the counter value will be replaced with the swapVal argument. The
    previous (old) value of the counter is returned.

  */
  unsigned long atomicCmpSwapCntGPI(const unsigned long cmpVal,const unsigned long swapVal,const unsigned int gpi_counter);

  //! Reset a global atomic counter (make it zero)
  /*! 
    
    \param gpi_counter the counter number to be reset-ed
    
    \return an int where 0 is success and -1 is operation failed
  */
  int atomicResetCntGPI(const unsigned int gpi_counter);

  /*@}*/

  //! \name Global Lock
  /*@{*/
  //! Lock the global resource lock
  /*! 
    
    \return an int where 0 is success (got lock) and -1 is operation failed (did not got lock)

    The global resource lock is a lock (synchronization mechanism)
    that works globally. All nodes can use it to limit access to a
    shared resource using the lock-unlock semantics. Once a node got
    the lock it can be sure it is the only one. Since it is a global
    resource is should be used wisely and used in a relaxed manner
    (try not to busy-loop to get the lock).
  */
  int globalResourceLockGPI(void);

  //! Unlock the global resource lock
  /*! 

    \return an int where 0 is success and -1 is operation failed (not owner of the lock)

    Unlock the previously locked global resource lock. For this
    operation to be successful, the node has to own lock.
  */
  int globalResourceUnlockGPI(void);

  /*@}*/

  //! \name Communication errors
  /*@{*/
  //! Check for errors on a communication queue
  /*! 
    
    \param gpi_queue The queue to check for errors
    
    \return a byte vector that has # node elements. On each position of the vector: 1=error,0=nonerror
  */
  unsigned char *getErrorVectorGPI(const unsigned int gpi_queue);

  /*@}*/


  //! \name Number of events
  /*@{*/

  //! Get number of open DMA requests on a queue
  /*! 
    \param gpi_queue the queue number to check
    
    \return an int with the number of open requests or -1 on error

    A queue has a limit on the number of outstanding requests
    (currently 1024) that can be posted to it without entering an
    error state. A waitDmaGPI must be done before this limit to make
    sure requests are finished. This function returns the number of
    requests open that is, not processed by a waitDmaGPI.
  */
  int openDMARequestsGPI(const unsigned int gpi_queue);


  //! Get number of open passive DMA requests
  /*! 
    
    \return an int with the number of open requests or -1 on error

    The passive channel has a limit on the number of outstanding
    requests (currently 1024) that can be posted without calling a
    waitDmaPassiveGPI. This function returns the number of
    requests open that is, not processed by a waitDmaPassiveGPI.

  */
  int openDMAPassiveRequestsGPI();

  //fds of event channels
  int getChannelFdGPI(void);

  /*@}*/

#ifdef __cplusplus
}
#endif

#endif
