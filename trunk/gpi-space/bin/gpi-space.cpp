/*
 * The interface program  providing access to the global,  persistent memory. It
 * provides access to process containers via a UNIX sockets.
 *
 */

#include <fhglog/minimal.hpp>

#include <GPI/GPI.h>

#include <gpi-space/config/config.hpp>
#include <gpi-space/config/config_io.hpp>
#include <gpi-space/config/parser.hpp>

#include <stdio.h> // snprintf
#include <unistd.h> // getuid
#include <sys/types.h> // uid_t
#include <pwd.h> // getpwuid
#include <csignal>
#include <cassert>
#include <iostream>
#include <fstream>

static int MAX_OPEN_DMA_REQUESTS (1);
static int number_of_nodes (1);

static void master_shutdown_handler (int signal)
{
  LOG(INFO, "GPI master process terminating due to signal: " << signal);
  killProcsGPI();

  exit (0);
}

static void slave_shutdown_handler (int signal)
{
  LOG(INFO, "GPI slave process terminating due to signal: " << signal);
  shutdownGPI();
  exit (0);
}

static int check_node (const int rank, const unsigned short port)
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
        LOG(WARN, "another GPI binary is still running and blocking the port, trying to kill it now");
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

static int check_gpi_environment (int ac, char * av[])
{
  int errors (0);

  if (isMasterProcGPI( ac, av ))
  {
    LOG(INFO, "GPI environment check initiating");

    const unsigned short port (getPortGPI());

    // check a single node
    const int num_nodes (generateHostlistGPI());
    for (int rank (0); rank < num_nodes; ++rank)
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

enum gpi_space_error_t
  {
      GPI_NO_ERROR = 0
    , GPI_GENERAL_FAILURE = 1
    , GPI_CONFIG_ERROR = 2
    , GPI_CHECK_FAILED = 4
    , GPI_STARTUP_FAILED = 5
    , GPI_WRITE_DMA_FAILED = 9
    , GPI_PASSIVE_SEND_FAILED = 10
    , GPI_PASSIVE_RECV_FAILED = 11

    , GPI_INTERNAL_ERROR = 23
    , GPI_TIMEOUT = 42
  };

static int distribute_config (const gpi_space::config & cfg)
{
  LOG(DEBUG, "distributing config...");

  memcpy (getDmaMemPtrGPI(), &cfg, sizeof (cfg));

  int ec (GPI_NO_ERROR);

  // distribute to all
  int success_count (0);
  for (int n (1); n < number_of_nodes; ++n)
  {
    // this code will only execute if we have a really large number of nodes...
    if (openDMAPassiveRequestsGPI() == MAX_OPEN_DMA_REQUESTS)
    {
      ec = waitDmaPassiveGPI();
      if (ec < 0)
      {
        LOG(ERROR, "error during waitDmaPassiveGPI(): " << ec);
        return GPI_INTERNAL_ERROR;
      }
      else
      {
        success_count += ec;
      }
    }

    ec = sendDmaPassiveGPI( 0, sizeof(cfg), n );
    if (ec != 0)
    {
      LOG(ERROR, "could not initiate passive send node config to node: " << n << ": " << ec);
      return GPI_PASSIVE_SEND_FAILED;
    }
  }

  if (openDMAPassiveRequestsGPI () > 0)
  {
    ec = waitDmaPassiveGPI();
    if (ec < 1)
    {
      LOG(ERROR, "not all passive sends completed after wait(): " << ec);
      return GPI_PASSIVE_SEND_FAILED;
    }
    else
    {
      success_count += ec;
    }
  }

  LOG(INFO, "config sucessfully distributed to " << success_count << " nodes");
  return GPI_NO_ERROR;
}

static int receive_config (gpi_space::config & cfg)
{
  int src_rank (-2);
  int ec = recvDmaPassiveGPI (0, sizeof (gpi_space::config), &src_rank);
  if (ec == 0)
  {
    if (src_rank != 0)
    {
      return GPI_INTERNAL_ERROR;
    }
    else
    {
      memcpy (&cfg, getDmaMemPtrGPI(), sizeof (gpi_space::config));
    }
  }
  else
  {
    return GPI_PASSIVE_RECV_FAILED;
  }

  return GPI_NO_ERROR;
}

static int main_loop (const gpi_space::config & cfg, const int rank)
{
  static const std::string prompt ("Please type \"q\" followed by return to quit: ");

  LOG(INFO, "gpi-space on rank " << rank << " running");

  int rc (GPI_NO_ERROR);

  // fire up message handling threads...

  if (cfg.node.daemonize)
  {
    // daemonize...

    // wait for signals
  }
  else if (rank == 0)
  {
    bool done (false);

    do
    {
      std::cout << prompt;

      std::string line;
      std::getline (std::cin, line);
      if (line.empty())
        continue;

      char c = line[0];
      switch (c)
      {
      case 'q':
        done = true;
        rc = 0;
        break;
      case 'l':
        std::cerr << "list of attached processes:" << std::endl;
        std::cerr << "    implement me" << std::endl;
        break;
      case 'h':
      case '?':
        std::cerr << "list of supported commands:"             << std::endl;
        std::cerr                                              << std::endl;
        std::cerr << "    h|? - print this help"               << std::endl;
        std::cerr << "      p - list all attached processes"   << std::endl;
        std::cerr << "d [num] - detach process #num or first"  << std::endl;
        std::cerr << "      s - print statistics"              << std::endl;
        std::cerr << "      a - list allocations"              << std::endl;
        std::cerr << "f [num] - remove an allocation (or all)" << std::endl;
        break;
      default:
        std::cerr << "command not understood, please use \"h\"" << std::endl;
        break;
      }
    }  while (! done);
  }
  return rc;
}

static void init_config (gpi_space::config & cfg)
{
  // quick hack
  std::string user_name;
  {
    std::stringstream sstr;
    passwd * pw_entry (getpwuid(getuid()));
    if (pw_entry)
    {
      sstr << pw_entry->pw_name;
    }
    else
    {
      LOG(WARN, "could not lookup username from uid " << getuid() << ": " << errno);
      sstr << getuid();
    }
    user_name = sstr.str();
  }

  snprintf ( cfg.node.socket_path
           , gpi_space::MAX_PATH_LEN
           , "/var/tmp/GPI-Space-S-%s"
           , user_name.c_str()
           );
}

static int configure (const gpi_space::config & cfg)
{
  // configure logging, etc.
  return 0;
}

int main (int ac, char *av[])
{
  {
    for (int i (0) ; i < ac; ++i)
    {
      std::cerr << "   av[" << i << "] = " << av[i] << std::endl;
    }
  }

  int rc (GPI_NO_ERROR);

  gpi_space::config config;
  if (isMasterProcGPI (ac, av))
  {
    FHGLOG_SETUP (ac, av);

    signal (SIGINT,  master_shutdown_handler);
    signal (SIGTERM, master_shutdown_handler);

    // read config from file
    std::string config_file ("/etc/gpi-space.rc");
    if (ac > 1)
    {
      config_file = av[1];
    }
    LOG(TRACE, "reading config from: " << config_file);

    gpi_space::parser::config_parser_t cfg_parser;
    try
    {
      gpi_space::parser::parse (config_file, boost::ref(cfg_parser));
      init_config (config);
      config.load (cfg_parser);
    }
    catch (std::exception const & ex)
    {
      LOG(ERROR, "could not read config file: " << config_file << ": " << ex.what());
      return GPI_CONFIG_ERROR;
    }
    LOG(TRACE, "read config: " << std::endl << config);
  }

  rc = check_gpi_environment (ac, av);
  if (rc != 0)
  {
    killProcsGPI();
    shutdownGPI();
    return GPI_CHECK_FAILED;
  }

  rc = startGPI (ac, av, "", config.gpi.memory_size);
  if (rc != 0)
  {
    LOG(ERROR, "GPI startup failed with error: " << rc);
    killProcsGPI();
    return GPI_STARTUP_FAILED;
  }
  else
  {
    LOG(INFO, "GPI started: version: " << getVersionGPI());
  }

  // initialize (static) globals
  number_of_nodes = generateHostlistGPI();
  MAX_OPEN_DMA_REQUESTS = getQueueDepthGPI();
  const int rank = getRankGPI ();

  if (0 == rank)
  {
    signal (SIGINT,  master_shutdown_handler);
    signal (SIGTERM, master_shutdown_handler);

    rc = distribute_config (config);
  }
  else if (rank > 0)
  {
    signal (SIGINT,  slave_shutdown_handler);
    signal (SIGTERM, slave_shutdown_handler);

    rc = receive_config (config);
  }
  else
  {
    LOG(FATAL, "Bazinga! Something is very very wrong, rank = " << rank);
    rc = GPI_INTERNAL_ERROR;
  }

  // common code starts here

  if (0 == rc)
  {
    rc = configure(config);
    allReduceGPI (&rc, &rc, 1, GPI_MAX, GPI_INT);

    if (rc == 0)
    {
      rc = main_loop(config, rank);
    }
    else
    {
      LOG(ERROR, "configuration on at least one node failed!");
    }
  }

  barrierGPI();

  LOG(DEBUG, "gpi process (rank " << rank << ") terminated with exitcode " << rc);

  return rc;
}
