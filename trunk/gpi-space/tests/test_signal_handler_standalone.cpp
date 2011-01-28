#include <signal.h>
#include <pthread.h>

#include <fhglog/minimal.hpp>

#include <gpi-space/signal_handler.hpp>

static bool got_signal (false);

static int handle_signal (int)
{
  got_signal = true;
  return 0;
}

static int handle_term (int)
{
  gpi::signal::handler().stop();
  return 0;
}

int main ()
{
  sigset_t restrict;
  sigfillset (&restrict);
  pthread_sigmask (SIG_BLOCK, &restrict, 0);

  FHGLOG_SETUP();

  gpi::signal::handler().start ();

  gpi::signal::handler().connect (2, handle_term);
  gpi::signal::handler().connect (15, handle_term);

  gpi::signal::handler().join ();

  LOG_FLUSH ();

  return 0;
}
