#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <Pv4dVM4.h>
#include <Pv4dVMLogger.h>


#include "fvmSync.h"

static int commAllowed = 1;

//just send a byte
#define MAXLEN 128

/* lets use a max of ports per addr */
#define MAXPORT 20300
static int initialPort = 20200;

#define printf pv4d_printf

/* test implementation of fvm condition variables */

static char * generateAddress()
{
  char *buf = (char *)malloc(64);


  /* start at 224.0.0.3 */ 
  static in_addr_t genaddr = (in_addr_t) 0xe0000005; 

  if(initialPort == MAXPORT) 
    genaddr++;

  uint32_t net = htonl(genaddr);
  if (inet_ntop(AF_INET, &net, buf, 64) == NULL )
    {
      return NULL;
    }
  return buf;
}

static int generatePort()
{
  return ++initialPort;
}


/*******************  Mutexes **********************************/

int fvmMutexInit(fvmMutex_t *fvm_mutex)
{
  return pthread_mutex_init((pthread_mutex_t *)fvm_mutex, NULL);
}


int fvmMutexDestroy(fvmMutex_t *fvm_mutex)
{
  return pthread_mutex_destroy((pthread_mutex_t *) fvm_mutex);

}

/****************** Condition variables ***********************/
/* TODO: figure out if we really need the mutex */

/* initialization */
/* creates a multicast socket with the variable name to be used for signaling */
/* to keep the semantics of pthreads, a fvmMutex to limit acess to the socket */

int fvmCondInit(fvmCondVar_t *fvm_var)
{
  struct ip_mreq mreq;
  const char *tmp_addr;
  int tmp_port;

  fvm_var->sock = 0;
  int flag_on = 1;

  if ((fvm_var->sock = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    return -1;
  }


  if (setsockopt(fvm_var->sock, SOL_SOCKET, SO_REUSEADDR, &flag_on, sizeof(flag_on)) < 0) {
    return -1;
  }

  memset(&(fvm_var->addr), 0, sizeof(fvm_var->addr));
  fvm_var->addr.sin_family = AF_INET;

  tmp_addr = generateAddress();

  fvm_var->addr.sin_addr.s_addr = inet_addr(tmp_addr);

  tmp_port = generatePort();
  fvm_var->addr.sin_port = htons( tmp_port );

  if (bind(fvm_var->sock, (struct sockaddr *) &(fvm_var->addr), sizeof(fvm_var->addr)) < 0) {
    return -1;
  }
     
  mreq.imr_multiaddr.s_addr = inet_addr(tmp_addr);

  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
     
  if (setsockopt(fvm_var->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
    return -1;
  }

  return 0;

}

/* wait */
/* - select on multicast socket that awakes when there is a signal (data on socket) */
/* if sender is set, we wait until a message is coming from that particular node */
int fvmCondWait(fvmCondVar_t *fvm_var, fvmMutex_t *fvm_mutex, const char *sender)
{
  struct sockaddr_in from_addr;
  unsigned int from_len = sizeof(from_addr);      

  struct hostent *host = gethostbyname(sender);
  struct in_addr h_addr;

  fd_set rfds;  
  int nbytes;

  char buf[MAXLEN+1];

  static int recvSignals;

  h_addr.s_addr = *((unsigned long *) host->h_addr_list[0]);

  recvSignals++;
  pv4d_printf("FVM: Waiting signal %d from %s\n", recvSignals, sender);

  memset(buf,0,MAXLEN+1);

  FD_ZERO(&rfds);
  FD_SET(fvm_var->sock, &rfds);

  do
    {
      if(select(FD_SETSIZE, &rfds, NULL, NULL, NULL) == -1)
	{
	  return -1;
	}
      
      if ((nbytes=recvfrom(fvm_var->sock, 
			   buf, 
			   MAXLEN,
			   0,
			   (struct sockaddr *) &from_addr,
			   &from_len)) < 0) 
	{
	  return -1;
	}
      

      
    }
  while(from_addr.sin_addr.s_addr != h_addr.s_addr);
  
  pv4d_printf("FVM: Got signal!!\n");
  return 0;
}

/* signal */
/* - send the multicast packet */
/* TODO: CAVEAT can't put in too much pressure, signal might get lost */
/* for ex. if you try to signal a waiting variable in a loop  */

int fvmCondSignal(fvmCondVar_t *fvm_var)
{
  char buf = 'o';

  char message_to_send[MAXLEN];
  size_t send_len;

  static int numSignals;

  numSignals++;

  pv4d_printf("FVM: Sending signal %d to waiting allocators\n", numSignals);
  
  snprintf(message_to_send, sizeof(message_to_send), "%c", buf);

  send_len = strlen(message_to_send);
  
  if ((sendto(fvm_var->sock, message_to_send, send_len, 0,
	      (struct sockaddr *) &(fvm_var->addr),
	      sizeof(fvm_var->addr))) != send_len)
    {
      return -1;
    }
  
  memset(message_to_send, 0, sizeof(message_to_send));

  return 0;
}

int fvmCondDestroy(fvmCondVar_t *fvm_var)
{

  return close(fvm_var->sock);
}


int waitAllCommunication()
{
  int queue;

  for(queue=0;queue<8;queue++)
    waitDmaVM(queue);

  return 0;
}

void deferCommunication()
{
  pthread_mutex_lock(&mutex_communication);
  commAllowed = 0;
  pthread_mutex_unlock(&mutex_communication);
}

int isCommAllowed()
{
  int ret=0;
  pthread_mutex_lock(&mutex_communication);
  ret = commAllowed;
  pthread_mutex_unlock(&mutex_communication);

  return ret;
}

void allowCommunication()
{
  pthread_mutex_lock(&mutex_communication);

  commAllowed = 1;
  pthread_cond_signal(&cond_communication);
  pthread_mutex_unlock(&mutex_communication);


}

void waitCommAllowed()
{
  pthread_mutex_lock(&mutex_communication);
  pthread_cond_wait(&cond_communication,&mutex_communication);
  pthread_mutex_unlock(&mutex_communication);
}

