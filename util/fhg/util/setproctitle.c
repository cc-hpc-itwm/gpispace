#include "setproctitle.h"
#include "program_info.h"

#include <unistd.h>
#include <malloc.h>
#include <string.h>

int setproctitle (const char *title, int ac, char *const av [])
{
  char exe [4096];

  if (strcmp (title, av [0]) == 0)
  {
    return 0;
  }

  if (fhg_get_executable_path (exe, sizeof(exe)-1) < 0)
  {
    // fallback to av[0]
    strncpy (exe, av [0], sizeof (exe));
  }

  // create a copy of argv, since argv might reside on a read-only page
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
