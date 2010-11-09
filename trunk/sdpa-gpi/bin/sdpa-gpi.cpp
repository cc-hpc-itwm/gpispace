/*
 * The interface program  providing access to the global,  persistent memory. It
 * provides access to process containers via a UNIX sockets.
 *
 */

#include <fhglog/minimal.hpp>

#include <GPI.h>

#include <csignal>
#include <cassert>
#include <iostream>

void master_shutdown_handler (int signal)
{
  LOG(INFO, "GPI master process terminating due to signal: " << signal);
  killProcsGPI();

  exit (0);
}

void slave_shutdown_handler (int signal)
{
  LOG(INFO, "GPI slave process terminating due to signal: " << signal);
  shutdownGPI();
  exit (0);
}

int check_node (const int rank, const unsigned short port)
{
  int errors (0);

  const char * host = getHostnameGPI (rank);

  LOG(DEBUG, "checking GPI on host := " << host << " rank := " << rank);

  if (pingDaemonGPI (host) != 0)
  {
    LOG(ERROR, "failed to ping GPI daemon on host := " << host << " (rank " << rank << ")");
    ++errors;
  }
  else
  {
    int rc (0);

    rc = checkPortGPI (host, port);
    if (rc != 0)
    {
      LOG(ERROR, "failed to check port " << port << " on " << host << " with error " << rc);
      ++errors;

      if (findProcGPI (host) == 0)
      {
        LOG(WARN, "another GPI binary is still running and blocking port, trying to kill it now");
        if (killProcsGPI() == 0)
        {
          LOG(INFO, "successfully killed old processes");
          --errors;
        }
      }
    }

    rc = checkSharedLibsGPI(host);
    if (rc != 0)
    {
      LOG(ERROR, "shared library requirements could not be met on host " << host << " rank " << rank << " with error " << rc);
      ++errors;
    }

    rc = runIBTestGPI (host);
    if (rc != 0)
    {
      LOG(ERROR, "InfiniBand test failed for host " << host << " rank " << rank << " with error " << rc);
      ++errors;
    }
  }

  return errors;
}

int check_gpi_environment (int ac, char * av[])
{
  int errors (0);

  if (isMasterProcGPI( ac, av ))
  {
    LOG(INFO, "GPI environment check initiating");

    const int number_of_nodes (generateHostlistGPI());
    const unsigned short port (getPortGPI());

    // check a single node
    for (int rank (0); rank < number_of_nodes; ++rank)
    {
      errors += check_node (rank, port);
    }

    if (errors)
    {
      LOG(ERROR, "GPI environment check failed with " << errors << " error(s)");
    }
    else
    {
      LOG(INFO, "GPI environment successful");
    }
  }

  return errors;
}

static const int EXIT_GENERAL_FAILURE = 1;
static const int EXIT_GPI_CHECK_FAILED = 4;
static const int EXIT_GPI_ERROR = 8;

int main (int ac, char *av[])
{
  // TODO:
  //    configure fhglog manually!!!
  //    environment variables do not work on the slaves
  // should  be sufficient  to just  transmit  the location  information of  the
  // logserver and a log-level
  FHGLOG_SETUP (ac, av);

  int rc = 0;

  rc = check_gpi_environment (ac, av);
  if (rc != 0)
  {
    killProcsGPI();
    shutdownGPI();
    return EXIT_GPI_CHECK_FAILED;
  }

  rc = startGPI (ac, av, "", 1024 << 20);
  if (rc != 0)
  {
    LOG(ERROR, "GPI startup failed with error: " << rc);
    killProcsGPI();
    return EXIT_GPI_ERROR;
  }
  else
  {
    LOG(INFO, "GPI started: version: " << getVersionGPI());
  }

  const int rank = getRankGPI ();

  if (rank < 0)
  {
    LOG(ERROR, "something is very very wrong, rank = " << rank);
    return EXIT_GPI_ERROR;
  }

  sighandler_t shutdown_handler = &slave_shutdown_handler;
  if (0 == rank)
  {
    shutdown_handler = &master_shutdown_handler;
  }

  signal (SIGINT,  shutdown_handler);
  signal (SIGTERM, shutdown_handler);

  if (0 == rank)
  {
    LOG(INFO, "master");

    // wait for user to terminate me
    char c;
    std::cin >> c;
  }
  else if (rank < 0)
  {
    LOG(FATAL, "Bazinga! rank = " << rank);
  }
  else
  {
    LOG(INFO, "slave " << rank);
  }

  barrierGPI ();

  return 0;
}
