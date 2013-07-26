#include "setproctitle.h"

#include <unistd.h>
#include <malloc.h>
#include <string.h>

int setproctitle (const char *title, int ac, char *const av [])
{
  char exe [4096];
  memset (exe, 0, sizeof(exe));

  if (strcmp (title, av [0]) == 0)
  {
    return 0;
  }

#ifdef __linux__
  if (readlink ("/proc/self/exe", exe, sizeof(exe)-1) < 0)
  {
    strncpy (exe, av [0], sizeof (exe));
  }
#else
# error "please implement 'setproctitle (title, argc, argv)' for your OS"
#endif

  // create a copy of argv, might reside on a read-only page
  char ** new_argv = (char **)malloc ( (ac+1) * sizeof (char*));
  int i = 0;
  new_argv [0] = strdup (title);
  for (i = 1; i < ac; ++i)
  {
    new_argv [i] = strdup (av [i]);
  }
  new_argv [i] = 0;

  return execvp (exe, new_argv);
}
