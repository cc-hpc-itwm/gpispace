#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <GPI.h>

void sig_alarm(int)
{
	killProcsGPI();
	exit(0);
}
int main (int ac, char * av[])
{
  if (ac > 1)
  {
    printf( "UID=%d EUID=%d GID=%d EGID=%d\n"
          , getuid(), geteuid()
	  , getgid(), getegid()
	  );

	  signal(SIGALRM, sig_alarm);
	  alarm(1);
	  startGPI(1, av, av[1], 1<<10);
	  alarm(0);
	  return EXIT_SUCCESS;
  }
  else
  {
	  fprintf(stderr, "usage: gpi-hack <binary>\n");
	  return EXIT_FAILURE;
  }
}
