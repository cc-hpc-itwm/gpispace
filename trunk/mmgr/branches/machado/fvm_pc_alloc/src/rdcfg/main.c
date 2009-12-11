
#include <fvmConfig.h>
#include <rdcfg.tab.h>
#include <stdlib.h>
#include <stdio.h>

int
main ()
{
  int ret = readConfigFile ("test.cfg");

  printf ("return code of yyparse = %d\n", ret);

  if (!(ret < 0))
    {
      printf ("SHMSZ: %ld\n", cfg.shmemsize);
      printf ("FVMSZ: %ld\n", cfg.fvmsize);
      printf ("MSQFILE: %s\n", cfg.msqfile);
      printf ("SHMFILE: %s\n", cfg.shmemfile);
    }

  return EXIT_SUCCESS;
}
