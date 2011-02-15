#include <signal.h>
#include <pthread.h>

#include <fhglog/minimal.hpp>

#include <gpi-space/signal_handler.hpp>

gpi::signal::handler_t::connection_t con;

static int handle_int (int)
{
  con.disconnect ();
  return 0;
}

static int handle_term (int)
{
  gpi::signal::handler().stop();
  return 0;
}

int main ()
{
  FHGLOG_SETUP();

  gpi::signal::handler().start ();

  con = gpi::signal::handler().connect (SIGINT, handle_int);
  gpi::signal::handler().connect (SIGALRM, handle_term);
  gpi::signal::handler().connect (SIGTERM, handle_term);

  alarm (10);

  gpi::signal::handler().join ();

  return 0;
}
