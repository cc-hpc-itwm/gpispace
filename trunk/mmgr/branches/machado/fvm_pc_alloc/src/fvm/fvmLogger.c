#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>

#include <fvmLogger.h>


#define PORT_LOGGER	17823

/* TODO: move all loggin to fhglog */

extern int myrank;
static pthread_mutex_t loggerLock = PTHREAD_MUTEX_INITIALIZER;

static int log_master2console=1;

void fvm_printf(const char *fmt, ...){
  char buf[1024]; // MR: get rid of this constant
	struct sockaddr_in serverL,client;
	struct hostent *server_dataL;

	pthread_mutex_lock(&loggerLock);

	if((myrank == 0) && log_master2console){      

		memset(buf,0,1024);
		va_list ap;
		//try to print in the allocated space
		va_start(ap,fmt);
		vsnprintf(buf,1020,fmt,ap);
		va_end(ap);

		fprintf(stdout,buf);

	}else{

		memset(buf,0,1024);
		va_list ap;
		//try to print in the allocated space
		va_start(ap,fmt);
		vsnprintf(buf,1020,fmt,ap);
		va_end(ap);

		//fprintf(stdout,buf);
  
		//call the logger
		int sockL;  
		if((sockL = socket(AF_INET,SOCK_DGRAM,0)) < 0){printf("socket error\n");}

		client.sin_family 			= AF_INET;
		client.sin_addr.s_addr 	= htonl(INADDR_ANY);
		client.sin_port 				= htons(0);

		int rc = bind(sockL,(struct sockaddr *)&client,sizeof(client));
		if(rc<0){printf("bind failed\n");}
  
		char hostStr[256];
		gethostname(hostStr,256);

		if((server_dataL = gethostbyname(hostStr)) == 0){printf("unknown host\n");}
		memcpy(&serverL.sin_addr,server_dataL->h_addr, server_dataL->h_length);
		serverL.sin_family = AF_INET;
		serverL.sin_port = htons(PORT_LOGGER);
  
		connect(sockL,(struct sockaddr *)&serverL,sizeof(serverL));
		sendto(sockL,buf,1024,0,(struct sockaddr *)&serverL,sizeof(serverL));
  
		close(sockL);

	}
  
	pthread_mutex_unlock(&loggerLock);

}
