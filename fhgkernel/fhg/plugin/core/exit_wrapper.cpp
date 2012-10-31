#include <unistd.h>
#include <dlfcn.h>
#include <stdio.h>
#include <signal.h>
#include <execinfo.h>

static void show_stackframe() {
  void *trace[16];
  char **messages = (char **)NULL;
  int i, trace_size = 0;

  trace_size = backtrace(trace, 16);
  messages = backtrace_symbols(trace, trace_size);
  printf("[bt] Execution path:\n");
  for (i=0; i < trace_size; ++i)
        printf("[bt] %s\n", messages[i]);
}

typedef void (*exit_handler_t)(int) __attribute__((noreturn));
static exit_handler_t real_exit = NULL;

#ifdef __cplusplus
extern "C" {
#endif

extern void exit (int ec);

#ifdef __cplusplus
}
#endif

void exit (int ec)
{
  char *msg;
  if (real_exit == NULL)
  {
    fprintf (stderr, "exit: wrapping exit %p\n", exit);
    fflush (stderr);

    *(void**)(&real_exit) = dlsym (RTLD_NEXT, "exit");

    fprintf (stderr, "exit: real_exit = %p\n", real_exit);
    fflush (stderr);

    if ((msg = dlerror ()) != NULL)
    {
      fprintf (stderr, "exit: dlopen failed: %s\n", msg);
      fflush (stderr);
      _exit (ec);
    }
  }

  show_stackframe();

  (*real_exit) (ec);
}
