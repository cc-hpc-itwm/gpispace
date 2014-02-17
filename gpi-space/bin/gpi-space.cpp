/*
 * The interface program  providing access to the global,  persistent memory. It
 * provides access to process containers via a UNIX sockets.
 *
 */

#include <stdio.h> // snprintf
#include <unistd.h> // getuid, alarm, setsid, fork
#include <sys/types.h> // uid_t

#include <csignal>
#include <cassert>
#include <iostream>
#include <fstream>

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <fhg/assert.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhgcom/kvs/kvsc.hpp>

#include <fhg/util/daemonize.hpp>
#include <fhg/util/pidfile_writer.hpp>
#include <fhg/revision.hpp>

#include <gpi-space/gpi/api.hpp>

#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/pc/container/manager.hpp>

static const char * program_name = "gpi-space";
static const int CONFIG_MAGIC = 0xdeadbeef;

#define MAX_PATH_LEN 4096
#define MAX_HOST_LEN 1024

struct config_t
{
  int  magic;
  char socket[MAX_PATH_LEN];
  char kvs_host[MAX_HOST_LEN];
  unsigned short kvs_port;
  unsigned int kvs_retry_count;
  char log_host[MAX_HOST_LEN];
  unsigned short log_port;
  char log_level;
} __attribute__((packed));
static config_t config;

static bool gpi_startup_done = false;
static bool stop_requested = false;

typedef gpi::api::gpi_api_t gpi_api_t;

// exit codes
static const int EX_USAGE = 2;
static const int EX_INVAL = 3;

static const int GPI_MAGIC_MISMATCH = 23;

static void initialize_config(config_t *c);
static void distribute_config_or_die(const config_t *c, gpi_api_t & gpi_api);
static void receive_config_or_die(config_t *c, gpi_api_t& gpi_api);

static void signal_handler (int sig);
namespace
{
  fhg::com::kvs::kvsc_ptr_t kvs_client;
}
static int configure_logging (const config_t *cfg, const char* logfile);
static int configure_kvs (const config_t *cfg);

int main (int ac, char *av[])
{
  int i = 0;
  bool daemonize = false;
  bool is_master = true;
  bool gpi_perform_checks = true;
  bool gpi_clear_caches = true;
  char pidfile[MAX_PATH_LEN];
  snprintf (pidfile, sizeof(pidfile), "%s", "");
  enum { API_auto, API_real, API_fake } requested_api;
  char socket_path[MAX_PATH_LEN];
  snprintf (socket_path, sizeof(socket_path), "/var/tmp");
  char logfile[MAX_PATH_LEN];
  memset (logfile, 0, sizeof(logfile));
  std::string default_memory_url ("gpi://?buffer_size=4194304&buffers=8");

  unsigned long long gpi_mem = (1<<26);
  unsigned short gpi_port = 0;
  unsigned int gpi_mtu = 0;
  int gpi_net = -1;
  int gpi_np = -1;
  int gpi_numa_socket = 0;
  unsigned int gpi_timeout = 120;

  std::vector<std::string> mem_urls;

  initialize_config (&config);

  // parse command line
  i = 1;
  while (i < ac)
  {
    if (strcmp(av[i], "--help") == 0 || strcmp(av[i], "-h") == 0)
    {
      ++i;
      fprintf(stderr, "%s: [options]\n", program_name);
      fprintf(stderr, "\n");
      fprintf(stderr, "      version: %s\n", fhg::project_version());
      fprintf(stderr, "     revision: %s\n", fhg::project_revision());
      fprintf(stderr, "\n");
      fprintf(stderr, "options\n");
      fprintf(stderr, "    --help|-h\n");
      fprintf(stderr, "      print this help information\n");
      fprintf(stderr, "    --version|-V\n");
      fprintf(stderr, "      print version information\n");
      fprintf(stderr, "\n");
      fprintf(stderr, "    --pidfile PATH (%s)\n", pidfile);
      fprintf(stderr, "      write master's PID to this file\n");
      fprintf(stderr, "\n");
      fprintf(stderr, "    --daemonize\n");
      fprintf(stderr, "      fork to background when all checks were ok\n");
      fprintf(stderr, "\n");
      fprintf(stderr, "    --socket PATH (%s)\n", socket_path);
      fprintf(stderr, "      create sockets in this base path\n");
      fprintf(stderr, "\n");
      fprintf(stderr, "    --mem-url URL (%s)\n", default_memory_url.c_str());
      fprintf(stderr, "      url of the default memory segment (1)\n");
      fprintf(stderr, "\n");
      fprintf(stderr, "      examples are:\n");
      fprintf(stderr, "         gpi://?buffers=8&buffer_size=4194304 GPI memory\n");
      fprintf(stderr, "         sfs://<path>?create=true&size=1073741824\n");
      fprintf(stderr, "\n");
      fprintf(stderr, "KVS options\n");
      fprintf(stderr, "    --kvs-host HOST (%s)\n", config.kvs_host);
      fprintf(stderr, "    --kvs-port PORT (%hu)\n", config.kvs_port);
      fprintf(stderr, "    --kvs-retry-count SIZE (%u)\n", config.kvs_retry_count);
      fprintf(stderr, "\n");
      fprintf(stderr, "LOG options\n");
      fprintf(stderr, "    --log-host HOST (%s)\n", config.log_host);
      fprintf(stderr, "    --log-port PORT (%hu)\n", config.log_port);
      fprintf(stderr, "    --log-level {T, D, I, W, E, F} (%c)\n", config.log_level);
      fprintf(stderr, "         _T_race, _D_ebug, _I_nfo\n");
      fprintf(stderr, "         _W_arn, _E_rror, _F_atal_\n");
      fprintf(stderr, "\n");
      fprintf(stderr, "GPI options\n");
      fprintf(stderr, "    --gpi-mem|-s BYTES (%llu)\n", gpi_mem);
      fprintf(stderr, "    --gpi-api STRING (%s)\n", "auto");
      fprintf(stderr, "      choose the GPI API to use\n");
      fprintf(stderr, "        fake - use the fake api\n");
      fprintf(stderr, "        real - use the real api\n");
      fprintf(stderr, "        auto - choose the best api\n");
      fprintf(stderr, "\n");
      fprintf(stderr, "GPI options (expert)\n");
      fprintf(stderr, "    --gpi-port|-p PORT (%hu)\n", gpi_port);
      fprintf(stderr, "    --gpi-np NUM (%d)\n", gpi_np);
      fprintf(stderr, "      number of processes to start\n");
      fprintf(stderr, "        -1 - one per host in hostlist\n");
      fprintf(stderr, "         N - only on the first N hosts\n");
      fprintf(stderr, "    --gpi-mtu BYTES (%u)\n", gpi_mtu);
      fprintf(stderr, "    --gpi-net TYPE (%d)\n", gpi_net);
      fprintf(stderr, "              0=IB\n");
      fprintf(stderr, "              1=ETH\n");
      fprintf(stderr, "    --gpi-timeout SECONDS (%u)\n", gpi_timeout);
      fprintf(stderr, "    --no-gpi-checks\n");
      fprintf(stderr, "      do not perform checks before gpi startup\n");
      fprintf(stderr, "    --no-clear-file-cache\n");
      fprintf(stderr, "      do not clear file caches on all nodes\n");
      exit(EXIT_SUCCESS);
    }
    else if (strcmp(av[i], "--version") == 0 || strcmp(av[i], "-V") == 0)
    {
      printf("%s\n", fhg::project_version());
      exit(EXIT_SUCCESS);
    }
    else if (strcmp(av[i], "--pidfile") == 0)
    {
      ++i;
      if (i < ac)
      {
        if ((strlen(av[i] + 1) > sizeof(pidfile)))
        {
          fprintf(stderr, "%s: path to pidfile is too large!\n", program_name);
          fprintf(stderr, "    at most %lu characters are supported\n", sizeof(pidfile));
          exit(EX_INVAL);
        }

        strncpy(pidfile, av[i], sizeof(pidfile));
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --pidfile\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--daemonize") == 0)
    {
      ++i;
      daemonize = true;
    }
    else if (strcmp(av[i], "--socket") == 0)
    {
      ++i;
      if (i < ac)
      {
        if ( ( (strlen(av[i]) + strlen("/S-gpi-space.XXXXX.XXXX") + 1)
             > sizeof(config.socket))
           )
        {
          fprintf(stderr, "%s: path to socket is too large!\n", program_name);
          fprintf(stderr, "    at most %lu characters are supported\n"
                 , sizeof(config.socket) - (strlen("/S-gpi-space.XXXXX.XXXX")+1)
                 );
          exit(EX_INVAL);
        }

        snprintf ( socket_path
                 , sizeof(socket_path)
                 , "%s"
                 , av[i]
                 );
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --socket\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--mem-url") == 0)
    {
      ++i;
      if (i < ac)
      {
        if ((strlen (av[i]) + 1) > MAX_PATH_LEN)
        {
          fprintf (stderr, "%s: memory url is too large!\n", program_name);
          fprintf (stderr, "    at most %d characters are supported\n"
                  , MAX_PATH_LEN - 1
                  );
          exit(EX_INVAL);
        }

        mem_urls.push_back (av [i]);

        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --mem-url\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--kvs-host") == 0)
    {
      ++i;
      if (i < ac)
      {
        if ((strlen(av[i] + 1) > sizeof(config.kvs_host)))
        {
          fprintf(stderr, "%s: hostname is too large!\n", program_name);
          fprintf(stderr, "    at most %lu characters are supported\n", sizeof(config.kvs_host));
          exit(EX_INVAL);
        }
        strncpy(config.kvs_host, av[i], sizeof(config.kvs_host));
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --kvs-host\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--kvs-port") == 0)
    {
      ++i;
      if (i < ac)
      {
        if (sscanf(av[i], "%hu", &config.kvs_port) == 0)
        {
          fprintf(stderr, "%s: kvs-port invalid: %s\n", program_name, av[i]);
          exit(EX_INVAL);
        }
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --kvs-port\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--kvs-retry-count") == 0)
    {
      ++i;
      if (i < ac)
      {
        if (sscanf(av[i], "%u", &config.kvs_retry_count) == 0)
        {
          fprintf(stderr, "%s: kvs-retry-count invalid: %s\n", program_name, av[i]);
          exit(EX_INVAL);
        }
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --kvs-retry-count\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--log-file") == 0)
    {
      ++i;
      if (i < ac)
      {
        if ((strlen(av[i] + 1) > sizeof(logfile)))
        {
          fprintf(stderr, "%s: logfile is too large!\n", program_name);
          fprintf(stderr, "    at most %lu characters are supported\n", sizeof(logfile));
          exit(EX_INVAL);
        }
        strncpy(logfile, av[i], sizeof(logfile));
        if (strlen (logfile) > 0)
        {
          setenv ("FHGLOG_to_file", logfile, true);
        }
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --log-file\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--log-host") == 0)
    {
      ++i;
      if (i < ac)
      {
        if ((strlen(av[i] + 1) > sizeof(config.log_host)))
        {
          fprintf(stderr, "%s: hostname is too large!\n", program_name);
          fprintf(stderr, "    at most %lu characters are supported\n", sizeof(config.log_host));
          exit(EX_INVAL);
        }
        strncpy(config.log_host, av[i], sizeof(config.log_host));
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --log-host\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--log-port") == 0)
    {
      ++i;
      if (i < ac)
      {
        if (sscanf(av[i], "%hu", &config.log_port) == 0)
        {
          fprintf(stderr, "%s: log-port invalid: %s\n", program_name, av[i]);
          exit(EX_INVAL);
        }
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --log-port\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--log-level") == 0)
    {
      ++i;
      if (i < ac)
      {
        config.log_level = av[i][0];
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --log-level\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--gpi-mem") == 0 || strcmp(av[i], "-s") == 0)
    {
      ++i;
      if (i < ac)
      {
        if (sscanf(av[i], "%llu", &gpi_mem) == 0)
        {
          fprintf(stderr, "%s: mem-size invalid: %s\n", program_name, av[i]);
          exit(EX_INVAL);
        }
        if (gpi_mem < sizeof(config_t))
        {
          fprintf (stderr
                  , "%s: memory size too small, at least %lu bytes required!\n"
                  , program_name
                  , sizeof(config_t)
                  );
          exit(EX_INVAL);
        }
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --gpi-mem\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--gpi-api") == 0)
    {
      ++i;
      if (i < ac)
      {
        requested_api = strcmp (av[i], "auto") == 0 ? API_auto
          : strcmp (av[i], "real") == 0 ? API_real
          : strcmp (av[i], "fake") == 0 ? API_fake
          : throw std::runtime_error
          ("invalid argument to --gpi-api: must be 'auto', 'real' or 'fake'");
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --gpi-api\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--gpi-port") == 0 || strcmp(av[i], "-p") == 0)
    {
      ++i;
      if (i < ac)
      {
        if (sscanf(av[i], "%hu", &gpi_port) == 0)
        {
          fprintf(stderr, "%s: gpi-port invalid: %s\n", program_name, av[i]);
          exit(EX_INVAL);
        }
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --gpi-port\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--gpi-mtu") == 0)
    {
      ++i;
      if (i < ac)
      {
        if (sscanf(av[i], "%u", &gpi_mtu) == 0)
        {
          fprintf(stderr, "%s: gpi-mtu invalid: %s\n", program_name, av[i]);
          exit(EX_INVAL);
        }
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --gpi-mtu\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--gpi-net") == 0)
    {
      ++i;
      if (i < ac)
      {
        if (sscanf(av[i], "%d", &gpi_net) == 0)
        {
          fprintf(stderr, "%s: gpi-net invalid: %s\n", program_name, av[i]);
          exit(EX_INVAL);
        }
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --gpi-net\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--gpi-timeout") == 0)
    {
      ++i;
      if (i < ac)
      {
        if (sscanf(av[i], "%u", &gpi_timeout) == 0)
        {
          fprintf(stderr, "%s: gpi-timeout invalid: %s\n", program_name, av[i]);
          exit(EX_INVAL);
        }
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --gpi-timeout\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--gpi-np") == 0)
    {
      ++i;
      if (i < ac)
      {
        if (sscanf(av[i], "%d", &gpi_np) == 0)
        {
          fprintf(stderr, "%s: gpi-np invalid: %s\n", program_name, av[i]);
          exit(EX_INVAL);
        }
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --gpi-np\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--no-gpi-checks") == 0)
    {
      ++i;
      gpi_perform_checks = false;
    }
    else if (strcmp(av[i], "--no-clear-file-cache") == 0)
    {
      ++i;
      gpi_clear_caches = false;
    }
    else if (strcmp(av[i], "--") == 0)
    {
      ++i;
      break;
    }
    else if (strcmp(av[i], "-t") == 0)
    {
      ++i;
      if (i < ac)
      {
        if (strcmp(av[i], "GPI_WORKER") == 0)
        {
          is_master = false;
          ++i;
        }
        else
        {
          fprintf(stderr, "%s: invalid GPI worker type: %s\n", program_name, av[i]);
          exit(EX_INVAL);
        }
      }
    }
    else
    {
      fprintf(stderr, "%s: unknown option: %s\n", program_name, av[i]);
      ++i;
    }
  }

  FHGLOG_SETUP ();

  snprintf ( config.socket
           , sizeof(config.socket)
           , "%s/S-gpi-space.%d.%d"
           , socket_path
           , getuid()
           , gpi_numa_socket
           );

  // initialize gpi api
  if (requested_api == API_auto)
  {
    try
    {
      gpi_api_t::create (gpi_api_t::REAL_API, is_master);
    }
    catch (gpi::exception::gpi_error const& ex)
    {
      fprintf (stderr, "%s: %s\n", program_name, ex.what());
      fprintf (stderr, "%s: fallback to fake API\n", program_name);

      gpi_api_t::destroy();
      gpi_api_t::create (gpi_api_t::FAKE_API, is_master);
    }
  }
  else if (requested_api == API_real)
  {
    gpi_api_t::create (gpi_api_t::REAL_API, is_master);
  }
  else if (requested_api == API_fake)
  {
    gpi_api_t::create (gpi_api_t::FAKE_API, is_master);
  }

  gpi_api_t & gpi_api = gpi_api_t::get();

  gpi_api.set_binary_path (av[0]);
  gpi_api.set_memory_size (gpi_mem);

  if (gpi_port)
  {
    gpi_api.set_port (gpi_port);
  }

  if (gpi_mtu)
  {
    gpi_api.set_mtu (gpi_mtu);
  }

  if (gpi_np != -1)
  {
    gpi_api.set_number_of_processes (gpi_np);
  }

  if (gpi_net != -1)
  {
    gpi_api.set_network_type ((gpi::network_type_t)gpi_net);
  }

  if (gpi_api.is_master())
  {
    LOG(INFO, "GPISpace version: " << fhg::project_version());
    LOG(INFO, "GPISpace revision: " << fhg::project_revision());
    LOG(INFO, "GPIApi version: " << gpi_api.version());

    if (0 != configure_kvs (&config))
    {
      LOG( ERROR
         , "could not configure KVS at: [" << config.kvs_host << "]:" << config.kvs_port
         );
      exit(EXIT_FAILURE);
    }

    if (gpi_perform_checks)
    {
      try
      {
        gpi_api.check();
      }
      catch (std::exception const & ex)
      {
        LOG(ERROR, "*** gpi check failed: " << ex.what());
        exit (EXIT_FAILURE);
      }
    }

    if (gpi_clear_caches)
    {
      gpi_api.clear_caches();
    }

    // quick hack to delete old kvs entries
    // TODO: find a better place
    for (std::size_t rnk = 0 ; rnk < gpi_api.number_of_nodes (); ++rnk)
    {
      std::string peer_name = fhg::com::p2p::to_string
        (fhg::com::p2p::address_t ("gpi-"+boost::lexical_cast<std::string>(rnk)));
      std::string kvs_key = "p2p.peer." + peer_name;
      kvs_client->del (kvs_key);
    }

    if (0 != strlen (pidfile))
    {
      try
      {
        fhg::util::pidfile_writer const pidfile_writer (pidfile);

        if (daemonize)
        {
          fhg::util::fork_and_daemonize_child_and_abandon_parent();
        }

        pidfile_writer.write();
      }
      catch (const std::exception& ex)
      {
        LOG (ERROR, ex.what());
        exit (EXIT_FAILURE);
      }
    }
    else
    {
      if (daemonize)
      {
        fhg::util::fork_and_daemonize_child_and_abandon_parent();
      }
    }
  }

  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGALRM, signal_handler);
  signal(SIGHUP, signal_handler);
  signal(SIGCHLD, signal_handler);

  try
  {
    gpi_startup_done = false;
    gpi_api.start (ac, av, gpi_timeout);
    gpi_startup_done = true;
  }
  catch (std::exception const & ex)
  {
    LOG(ERROR, "GPI could not be started: " << ex.what());
    return EXIT_FAILURE;
  }
  LOG(INFO, "GPI started");

  if (gpi_api.is_master())
  {
    config.magic = CONFIG_MAGIC;
    distribute_config_or_die (&config, gpi_api);
  }
  else
  {
    receive_config_or_die (&config, gpi_api);
    if (CONFIG_MAGIC != config.magic)
    {
      exit(GPI_MAGIC_MISMATCH);
    }
  }

  if (0 != configure_logging (&config, logfile))
  {
    LOG(WARN, "could not setup logging");
  }

  int ec = configure_kvs (&config);
  if (ec != 0)
  {
    LOG(ERROR, "could not connect to KVS: " << strerror(ec));
    exit(EXIT_FAILURE);
  }

  try
  {
    stop_requested = false;

    LOG( TRACE
       ,  "rank=" << gpi_api.rank()
       << " dma=" << gpi_api.dma_ptr()
       << " #nodes=" << gpi_api.number_of_nodes()
       << " mem_size=" << gpi_api.memory_size()
       );

    if (mem_urls.empty ())
      mem_urls.push_back (default_memory_url);
    gpi::pc::container::manager_t *global_container_mgr
      ( new gpi::pc::container::manager_t
        (config.socket, mem_urls, gpi_api, kvs_client)
      );

    LOG(INFO, "started GPI interface on rank " << gpi_api.rank() << " at " << config.socket);

    if (0 == gpi_api.rank())
    {
      if (isatty(0) && isatty(1))
      {
        static const std::string prompt
          ("Please type \"q\" followed by return to quit: ");

        bool done = false;
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
          case 'h':
          case '?':
            std::cerr << "list of supported commands:"             << std::endl;
            std::cerr                                              << std::endl;
            std::cerr << "    h|? - print this help"               << std::endl;
            std::cerr << "      q - quit"                          << std::endl;

            break;
          default:
            std::cerr << "command not understood, please use \"h\"" << std::endl;
            break;
          }
        }
      }
      else
      {
        do { } while (!stop_requested && pause() == -1 && errno == EINTR);
      }
    }
    else
    {
      do { } while (!stop_requested && pause() == -1 && errno == EINTR);
    }

    ec = EXIT_SUCCESS;
    LOG(INFO, "gpi process (rank " << gpi_api.rank() << ") terminated with exitcode: " << ec);
  }
  catch (std::exception const & ex)
  {
    ec = EXIT_FAILURE;
    LOG(ERROR, "gpi process (rank " << gpi_api.rank() << ") failed: " << ex.what());
  }

  return ec;
}

static void signal_handler (int sig)
{
  switch (sig)
  {
  case SIGALRM:
    if (not gpi_startup_done)
    {
      LOG(ERROR, "startup failed");
      exit (1);
    }
    else
    {
    }
    break;
  case SIGTERM:
    close(0);
  case SIGINT:
    stop_requested = true;
    break;
  default:
    break;
  }
}

static int configure_logging (const config_t *cfg, const char* logfile)
{
  char server_url[MAX_HOST_LEN + 32];

  // logging oonfiguration
  snprintf( server_url, sizeof(server_url), "%s:%hu"
          , cfg->log_host, cfg->log_port
          );
  const char *log_level = 0;
  switch (cfg->log_level)
  {
  case 'T':
    log_level = "TRACE";
    break;
  case 'D':
    log_level = "DEBUG";
    break;
  case 'I':
    log_level = "INFO";
    break;
  case 'W':
    log_level = "WARN";
    break;
  case 'E':
    log_level = "ERROR";
    break;
  case 'F':
    log_level = "FATAL";
    break;
  }

  setenv("FHGLOG_to_server", server_url, true);
  if (log_level)
  {
    setenv("FHGLOG_level", log_level, true);
  }
  setenv ("FHGLOG_to_console", "stderr", true);

  if (strlen (logfile) > 0)
  {
    setenv ("FHGLOG_to_file", logfile, true);
  }

  FHGLOG_SETUP();

  return 0;
}

static int configure_kvs (const config_t *cfg)
{
  try
  {
    kvs_client = fhg::com::kvs::kvsc_ptr_t
      ( new fhg::com::kvs::client::kvsc ( cfg->kvs_host
                                        , boost::lexical_cast<std::string>(cfg->kvs_port)
                                        , true
                                        , boost::posix_time::seconds(1)
                                        , cfg->kvs_retry_count
                                        )
      );

    // workaround until we have the above structure
    // put/del some entry to check the connection
    fhg::com::kvs::scoped_entry_t
      ( kvs_client
      , "kvs.connection.check"
      , "dummy value"
      );
  }
  catch (std::exception const &ex)
  {
    return -ESRCH;
  }
  return 0;
}

static void initialize_config (config_t * c)
{
  memset(c, 0, sizeof(config_t));

  if (gethostname (c->kvs_host, MAX_HOST_LEN) != 0)
  {
    snprintf ( c->kvs_host
             , sizeof(c->kvs_host)
             , "localhost"
             );
  }
  c->kvs_port = 2439;
  c->kvs_retry_count = 2;

  if (gethostname (c->log_host, MAX_HOST_LEN) != 0)
  {
    snprintf ( c->log_host
             , MAX_HOST_LEN
             , "localhost"
             );
  }
  c->log_port = 2438;
  c->log_level = 'I';
}

static void distribute_config_or_die(const config_t *c, gpi_api_t& gpi_api)
{
  memcpy(gpi_api.dma_ptr(), c, sizeof(config_t));
  const size_t max_enqueued_requests (gpi_api.queue_depth());
  for (size_t rank (1); rank < gpi_api.number_of_nodes(); ++rank)
  {
    if (gpi_api.open_passive_requests () >= max_enqueued_requests)
    {
      try
      {
        gpi_api.wait_passive ();
      }
      catch (std::exception const & ex)
      {
        LOG(ERROR, "could not wait on passive channel: " << ex.what());
        gpi_api.kill();
        exit(EXIT_FAILURE);
      }
    }

    try
    {
      gpi_api.send_passive( 0, sizeof(config_t), rank );
    }
    catch (std::exception const & ex)
    {
      LOG(ERROR, "could not send config to: " << rank << ": " << ex.what());
      gpi_api.kill();
      exit(EXIT_FAILURE);
    }
  }

  try
  {
    gpi_api.wait_passive ();
  }
  catch (std::exception const & ex)
  {
    LOG(ERROR, "could not wait on passive channel: " << ex.what());
    gpi_api.kill();
    exit(EXIT_FAILURE);
  }
  memset(gpi_api.dma_ptr(), 0, sizeof(config_t));
}

static void receive_config_or_die(config_t *c, gpi_api_t& gpi_api)
{
  gpi::rank_t source = 0;

  try
  {
    gpi_api.recv_passive( 0, sizeof(config_t), source );
    if (0 != source)
    {
      LOG(ERROR, "got passive dma message from unexpected source: " << source);
      throw std::runtime_error
        ("got passive dma message from unexpected source");
    }
  }
  catch (std::exception const & ex)
  {
    LOG(ERROR, "could not receive passive dma message: " << ex.what());
    exit(EXIT_FAILURE);
  }

  memcpy(c, gpi_api.dma_ptr(), sizeof(config_t));
  memset(gpi_api.dma_ptr(), 0, sizeof(config_t));
}
