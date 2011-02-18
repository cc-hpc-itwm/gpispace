/*
 * The interface program  providing access to the global,  persistent memory. It
 * provides access to process containers via a UNIX sockets.
 *
 */

#include <stdio.h> // snprintf
#include <unistd.h> // getuid, alarm
#include <sys/types.h> // uid_t
#include <pwd.h> // getpwuid

#include <csignal>
#include <cassert>
#include <iostream>
#include <fstream>

#include <fhglog/minimal.hpp>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <gpi-space/config/config.hpp>
#include <gpi-space/config/config_io.hpp>
#include <gpi-space/config/parser.hpp>
#include <gpi-space/signal_handler.hpp>
#include <gpi-space/gpi/api.hpp>

#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/pc/container/manager.hpp>

typedef gpi::api::gpi_api_t gpi_api_t;

static int shutdown_handler (gpi_api_t * api, int signal)
{
  LOG(INFO, "GPI process (rank " << api->rank() << ") terminating due to signal: " << signal);
  FHGLOG_FLUSH();
  api->shutdown ();
  exit (0);
}

static int suspend_handler (gpi_api_t * api, int signal)
{
  LOG(INFO, "GPI process (rank " << api->rank() << ") suspended due to signal: " << signal);
  ::raise (SIGSTOP);
  return 0;
}

static int resume_handler (gpi_api_t * api, int signal)
{
  LOG(INFO, "GPI process (rank " << api->rank() << ") resumed due to signal: " << signal);
  return 0;
}

static void distribute_config (const gpi_space::config & cfg, gpi_api_t & gpi_api)
{
  LOG(DEBUG, "distributing config...");

  memcpy (gpi_api.dma_ptr(), &cfg, sizeof (cfg));

  const gpi::queue_desc_t queue(0);

  // distribute to all
  gpi::size_t success_count (0);
  for (gpi::rank_t n (1); n < gpi_api.number_of_nodes(); ++n)
  {
    gpi_api.write_dma (0, 0, sizeof(cfg), n, queue);

    if (gpi_api.max_dma_requests_reached(queue))
    {
      success_count += gpi_api.wait_dma (queue);
    }
  }
  success_count += gpi_api.wait_dma (queue);

  LOG(INFO, "config successfully distributed to " << success_count << " nodes");

  gpi_api.barrier ();
}

static void receive_config (gpi_space::config & cfg, gpi_api_t & gpi_api)
{
  gpi_api.barrier ();
  memcpy (&cfg, gpi_api.dma_ptr(), sizeof (cfg));
}

static void configure (const gpi_space::config & cfg)
{
  LOG(INFO, "configuring...");
  gpi_space::logging::configure (cfg.logging);
}

static int main_loop (const gpi_space::config & cfg, const gpi::rank_t rank)
{
  configure(cfg);

  static const std::string prompt ("Please type \"q\" followed by return to quit: ");

  LOG(INFO, "gpi-space on rank " << rank << " running");

  // create socket path and generate a process specific socket
  namespace fs = boost::filesystem;
  fs::path socket_path (cfg.gpi.socket_path);
  {
    fs::create_directories (socket_path);
    chmod (socket_path.string().c_str(), 01777);

    socket_path /= ("GPISpace-" + boost::lexical_cast<std::string>(getuid()));
    fs::create_directories (socket_path);
    chmod (socket_path.string().c_str(), 00700);

    socket_path /= cfg.gpi.socket_name;
  }

  gpi::pc::container::manager_t mgr (socket_path.string());
  mgr.start ();

  // fire up message handling threads...

  //   - passive receive thread
  //       - hand message over to central daemon (N handler threads)
  //       - handle it
  //   - socket thread(s)
  //       - attach process container interface
  //       - attach control interface
  //       - both  use  the  same  backend  but  use  different  protocols  to
  //         communicate with the "client"
  //
  // passive -> +-------+
  // control -> | queue | -> central worker pool
  // process -> +-------+
  //
  //    work item:
  //        - type
  //        - input data
  //        - output data
  //        - callback function - void (work::ptr)
  //

  if (cfg.node.daemonize)
  {
    // daemonize...

    // wait for signals
  }
  else if (rank == 0)
  {
    bool done (false);

    while (std::cin.good() && !done)
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
    };
  }
  else
  {
    gpi::signal::handler().join ();
  }

  mgr.stop ();

  return EXIT_SUCCESS;
}

static void init_config (gpi_space::config & cfg)
{
  cfg.gpi.timeout_in_sec = 0;
}

int main (int ac, char *av[])
{
  gpi::signal::handler().start();

  {
    for (int i (0) ; i < ac; ++i)
    {
      std::cerr << "   av[" << i << "] = " << av[i] << std::endl;
    }
  }

  gpi_space::config config;

  // initialize gpi api
  gpi_api_t & gpi_api (gpi_api_t::create (ac, av));

  if (gpi_api.is_master ())
  {
    FHGLOG_SETUP (ac, av);

    // read config from file
    typedef std::vector<std::string> path_list_t;
    path_list_t search_path;
    search_path.push_back ("/etc/gpi.rc");

    // user config
    {
      long pwent_size (sysconf(_SC_GETPW_R_SIZE_MAX));
      if (pwent_size > 0)
      {
        char *buf = new char[pwent_size];
        struct passwd pwent;
        struct passwd *result (0);
        int ec (0);
        if ((ec = getpwuid_r(getuid(), &pwent, buf, pwent_size, &result)) == 0)
        {
          if (result)
          {
            std::string home_dir (pwent.pw_dir);
            search_path.push_back (home_dir + "/.sdpa/configs/gpi.rc");
          }
          else
          {
            LOG(WARN, "could not retrieve passwd entry for uid: " << getuid());
          }
        }
        else
        {
          LOG(ERROR, "an error occured while retrieving passwd entry for uid: " << getuid() << " = " << ec);
        }
        delete [] buf;
      }
      else
      {
        LOG(WARN, "could not get _SC_GETPW_R_SIZE_MAX from sysconf");
      }
    }

    if (ac > 1)
    {
      search_path.push_back (av[1]);
    }

    gpi_space::parser::config_parser_t cfg_parser;

    std::map <std::string, bool> files_seen;
    for ( path_list_t::const_iterator p (search_path.begin())
        ; p != search_path.end()
        ; ++p
        )
    {
      const std::string config_file (*p);
      if (files_seen.find (config_file) == files_seen.end())
      {
        LOG(TRACE, "trying to read config from: " << config_file);
        try
        {
          gpi_space::parser::parse (config_file, boost::ref(cfg_parser));
          files_seen[config_file] = true;
        }
        catch (std::exception const & ex)
        {
          LOG(WARN, "could not read config file: " << config_file << ": " << ex.what());
          continue;
        }
      }
    }

    try
    {
      init_config (config);
      config.load (cfg_parser);
    }
    catch (std::exception const & ex)
    {
      LOG(ERROR, "could not configure: " << ex.what());
      return EXIT_FAILURE;
    }
  }

  if (gpi_api.is_master ())
  {
    gpi_api.check();
  }

  gpi_api.set_memory_size (config.gpi.memory_size);
  gpi_api.start (config.gpi.timeout_in_sec);
  LOG(INFO, "GPI started: version: " << gpi_api.version());

  gpi::signal::handler().connect
    (SIGINT, boost::bind (shutdown_handler, &gpi_api, _1));
  gpi::signal::handler().connect
    (SIGTERM, boost::bind (shutdown_handler, &gpi_api, _1));
  gpi::signal::handler().connect
    (SIGTSTP, boost::bind (suspend_handler, &gpi_api, _1));
  gpi::signal::handler().connect
    (SIGCONT, boost::bind (resume_handler, &gpi_api, _1));

  if (gpi_api.is_master())
  {
    distribute_config (config, gpi_api);
  }
  else
  {
    receive_config (config, gpi_api);
  }

  gpi_api.barrier();

  int rc (EXIT_SUCCESS);
  try
  {
    rc = main_loop(config, gpi_api.rank());
    LOG(INFO, "gpi process (rank " << gpi_api.rank() << ") terminated with exitcode: " << rc);
  }
  catch (std::exception const & ex)
  {
    rc = EXIT_FAILURE;
    LOG(ERROR, "gpi process (rank " << gpi_api.rank() << ") failed: " << ex.what());
  }

  gpi_api.shutdown ();

  gpi::signal::handler().stop();
  return rc;
}
