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

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <fhglog/minimal.hpp>
#include <fhgcom/kvs/kvsc.hpp>

#include <gpi-space/version.hpp>
#include <gpi-space/config/config.hpp>
#include <gpi-space/config/config_io.hpp>
#include <gpi-space/config/parser.hpp>
#include <gpi-space/signal_handler.hpp>
#include <gpi-space/gpi/api.hpp>

#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/pc/container/manager.hpp>

typedef gpi::api::gpi_api_t gpi_api_t;

gpi::pc::container::manager_t *global_container_mgr(NULL);

static int shutdown_handler(int signal)
{
  if (global_container_mgr)
  {
    global_container_mgr->stop();
  }

  LOG(INFO, "GPISpace terminating due to signal: " << signal);
  FHGLOG_FLUSH();
  exit (-signal);
}

static int suspend_handler (int signal)
{
  LOG(INFO, "GPISpace process (" << gpi_api_t::get().rank() << ") suspended");
  ::raise (SIGSTOP);
  return 0;
}

static int resume_handler (int signal)
{
  LOG(INFO, "GPISpace process (" << gpi_api_t::get().rank() << ") resumed");
  return 0;
}

static void distribute_config (const gpi_space::config & cfg, gpi_api_t & gpi_api)
{
  DLOG(TRACE, "distributing config...");

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

  LOG(DEBUG, "config successfully distributed to " << success_count << " nodes");
}

static void receive_config (gpi_space::config & cfg, gpi_api_t & gpi_api)
{
  memcpy (&cfg, gpi_api.dma_ptr(), sizeof (cfg));
}

static void configure (const gpi_space::config & cfg)
{
  gpi_space::logging::configure (cfg.logging);
  gpi_space::node::configure (cfg.node);
}

static int main_loop (const gpi_space::config & cfg, const gpi::rank_t rank)
{
  configure(cfg);

  gpi_api_t & gpi_api (gpi_api_t::get());

  const std::string kvs_prefix
      ( "gpi.node."
      + boost::lexical_cast<std::string>(gpi_api.rank())
      );

  LOG( TRACE
     ,  "rank=" << gpi_api.rank()
     << " dma=" << gpi_api.dma_ptr()
     << " #nodes=" << gpi_api.number_of_nodes()
     << " mem_size=" << gpi_api.memory_size()
     );

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

  global_container_mgr = new gpi::pc::container::manager_t(socket_path.string());
  gpi::pc::container::manager_t & mgr = *global_container_mgr;
  mgr.start ();

  LOG(INFO, "started GPI interface at " << socket_path);

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
  else if (isatty (0) && isatty (1)) // usually on the master node only
  {
    // TODO: use the shell interface
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
    }
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

namespace
{
  static std::string get_home_dir()
  {
    struct passwd *pw(getpwuid(getuid()));
    if (pw && pw->pw_dir)
    {
      return pw->pw_dir;
    }
    return "/";
  }

  static std::string const & home_dir()
  {
    static std::string home(get_home_dir());
    return home;
  }

  static std::string expand_user(std::string const &arg)
  {
    std::string s(arg);
    std::string::size_type pos(s.find('~'));
    if (pos != std::string::npos)
    {
      s.replace(pos, 1, home_dir().c_str());
    }
    return s;
  }
}

int main (int ac, char *av[])
{
  gpi::signal::handler().start();

  FHGLOG_SETUP (ac, av);

  {
    for (int i (0) ; i < ac; ++i)
    {
      std::cerr << "   av[" << i << "] = " << av[i] << std::endl;
    }
  }

  // read config from file
  typedef std::vector<std::string> path_list_t;
  path_list_t search_path;
  search_path.push_back ("/etc/gpi.rc");
  search_path.push_back (expand_user("~/.sdpa/configs/gpi.rc"));

  gpi_space::parser::config_parser_t cfg_parser;
  {
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
  }

  gpi_space::config config;
  try
  {
    init_config (config);
    config.load (cfg_parser);
    gpi_space::logging::configure (config.logging);
  }
  catch (std::exception const & ex)
  {
    LOG(ERROR, "could not configure: " << ex.what());
    return EXIT_FAILURE;
  }

  // initialize gpi api
  gpi_api_t & gpi_api (gpi_api_t::create (cfg_parser.get("gpi.api", gpi_api_t::REAL_API)));
  gpi_api.init (ac, av);

  if (gpi_api.is_master())
  {
    LOG(INFO, "GPISpace version: " << gpi::version_string());
    LOG(INFO, "GPIApi version: " << gpi_api.version());
  }


  if (gpi_api.is_master ())
  {
    gpi_api.check();
  }

  gpi_api.set_memory_size (config.gpi.memory_size);

  try
  {
    gpi_api.start (config.gpi.timeout_in_sec);
  }
  catch (std::exception const & ex)
  {
    LOG(ERROR, "GPI could not be started: " << ex.what());
    return EXIT_FAILURE;
  }

  gpi::signal::handler().connect(SIGINT,  &shutdown_handler);
  gpi::signal::handler().connect(SIGTERM, &shutdown_handler);
  gpi::signal::handler().connect(SIGTSTP, &suspend_handler);
  gpi::signal::handler().connect(SIGCONT, &resume_handler);

  // unfortunately this is still required due to KVS!!!  if we have another way,
  // we don't have to distribute the config via this mechanism
  //
  // idea:
  //
  //    *one* file somewhere in $HOME that contains the url to the kvs
  //    connect to the kvs
  //    get everything else from there
  if (gpi_api.is_master())
  {
    distribute_config(config, gpi_api);
    gpi_api.barrier();
  }
  else
  {
    gpi_api.barrier();
    receive_config (config, gpi_api);
  }

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

  gpi::signal::handler().stop();
  return rc;
}
