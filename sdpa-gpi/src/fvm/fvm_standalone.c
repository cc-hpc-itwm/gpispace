#include <stdio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>

#include <Pv4dVMLogger.h>
#include <Pv4dVM4.h>

#include "fvm.h"
#include "fvmConfig.h"

void signal_handler(int sig){
        printf("Sig handler called...\n");
	shutdownPv4dVM();
	fvmLeave();
	switch (sig)
	{
		case SIGTERM:
		case SIGINT:
		case SIGHUP:
			exit(0);
			break;
		default:
			printf("Aborted (signal %d)\n", sig);
			exit(sig);
			break;
	}
}

void init_config_path (char * buf, size_t len)
{
  struct passwd * pw_entry = getpwuid (getuid());
  if (0 == pw_entry)
  {
    perror ("getpwuid");
    exit (1);
  }

  snprintf (buf, len, "%s/%s", pw_entry->pw_dir, ".sdpa/configs/sdpa-gpi.cfg");
}

int main(int argc, char *argv[])
{

  int ret,i;

  int nnodes;

  const char *hosts[1024];
  char configpath[4096];
  init_config_path (configpath, 4096);

  /*
#ifdef GPI_CONFIG_PATH
	const char * configpath = GPI_CONFIG_PATH; //hardcoded default
#else
#   error "GPI_CONFIG_PATH must be defined in order to compile this file!"
#endif
  */

  int optionsParse = 4;

  configFile_t config;

  signal( SIGINT, signal_handler );
  signal( SIGTERM, signal_handler );
  signal( SIGHUP, signal_handler );
  signal( SIGSEGV, signal_handler );
  signal( SIGABRT, signal_handler );

  //TODO: think about command line option
  config = readConfigFile(configpath, optionsParse);


  if(fvmInit(config)){
    printf("error init\n");
    return -1;
  }

  fflush (NULL);

  ret = startPv4dVM(argc, argv, "", config.fvmsize);
  if(ret !=0)
    return -1;

  fflush (NULL);

  pv4dBarrierVM();

  //get hostnames
  nnodes = getNodeCountVM();

  for(i=0;i<nnodes;i++){
    hosts[i] = getHostnameVM(i);
    pv4d_printf("host name %d : %s\n", hosts[i]);

  }

  fflush (NULL);

  //initialize allocator
  fvmMMInit(getDmaMemPtrVM(), config.fvmsize, getRankVM(), nnodes, &(hosts[0]));

  pv4dBarrierVM();

  pv4d_printf("Waiting for pc...\n");

  fflush (NULL);

  //wait for PC to connect
  while(!fvmWait4PC(config)) {

    //loop waiting for requests
    fvmListenRequests();

    pv4d_printf("Waiting for pc...\n");

    fflush (NULL);

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

  fflush (NULL);

  shutdownPv4dVM();
  fvmLeave();

  return 0;
}
