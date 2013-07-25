#include "setproctitle.h"

#include <unistd.h>
#include <malloc.h>
#include <string.h>

int setproctitle (const char *title, int argc, char *argv [])
{
  if (strcmp (argv [0], title) == 0)
  {
    return 0; // already changed
  }
  else
  {
    // create a copy of argv
    char ** new_argv = (char **)malloc ( (argc+1) * sizeof (char*));
    new_argv [0] = strdup (title);
    int i = 0;
    for (i = 1 ; i < argc ; ++i)
    {
      new_argv [i] = strdup (argv [i]);
    }
    new_argv [argc] = 0;

    return execvp (argv [0], new_argv);
  }
}
