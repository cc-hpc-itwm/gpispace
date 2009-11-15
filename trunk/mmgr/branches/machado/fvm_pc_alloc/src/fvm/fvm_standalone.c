#include <stdio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>


#include <Pv4dLogger.h>
#include <Pv4dVM4.h>

#include "fvm.h"
#include "fvmConfig.h"

void signal_handler(int sig){
        printf("Sig handler called...\n");
	killProcsVM();
	shutdownPv4dVM(); 
	fvmLeave();
	exit(0);                          
}


int main(int argc, char *argv[])
{

  int ret,i;

  int nnodes;

  const char *hosts[1024];

#ifdef FVM_CONFIG_PATH
	char * configpath = FVM_CONFIG_PATH; //hardcoded default
#else
	char * configpath = "/u/herc/machado/bin/fvmconfig"; //hardcoded default
#endif

  int optionsParse = 4;

  configFile_t config;

  signal(SIGINT,signal_handler);

  //TODO: think about command line option
  config = readConfigFile(configpath, optionsParse);


  if(fvmInit(config)){
    printf("error init\n");
    return -1;
  }

  ret = startPv4dVM(argc, argv, "", config.fvmsize);
  if(ret !=0)
    return -1;
 
  pv4dBarrierVM();

  //get hostnames 
  nnodes = getNodeCountVM();

  for(i=0;i<nnodes;i++){	
    hosts[i] = getHostnameVM(i);
    pv4d_printf("host name %d : %s\n", hosts[i]);

  }

  //initialize allocator
  fvmMMInit(getDmaMemPtrVM(), config.fvmsize, getRankVM(), nnodes, &(hosts[0]));


  pv4d_printf("Waiting for pc...\n");

  //wait for PC to connect
  while(!fvmWait4PC(config)) {
	  
    //loop waiting for requests
    fvmListenRequests();

    pv4d_printf("Waiting for pc...\n");

    //initialize to wait for another
    if(fvmInit(config))
      {
	printf("error init\n");
	return -1;
      }
  
	  
    /* 	  pv4d_printf("\nWaiting for the other nodes...\n"); */
    /* 	  pv4dBarrierVM(); */
  }
/*   else */
/*     pv4d_printf("Error waiting for pc\n"); */


  shutdownPv4dVM();

  return 0;
}
