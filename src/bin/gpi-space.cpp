/*
 * The interface program  providing access to the global,  persistent memory. It
 * provides access to process containers via a UNIX sockets.
 *
 */

#include <stdio.h> // snprintf
#include <unistd.h> // getuid, alarm, setsid, fork
#include <sys/types.h> // uid_t

#include <csignal>
#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhgcom/kvs/kvsc.hpp>

#include <fhg/util/make_unique.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/thread/event.hpp>
#include <fhg/revision.hpp>

#include <gpi-space/gpi/api.hpp>
#include <gpi-space/gpi/fake_api.hpp>
#include <gpi-space/gpi/gaspi.hpp>

#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/pc/container/manager.hpp>

#include <boost/optional.hpp>

#include <memory>

static const char * program_name = "gpi-space";
#define MAX_PATH_LEN 4096
#define MAX_HOST_LEN 1024

struct config_t
{
  config_t()
    : magic (0)
    // socket
    // kvs_host
    , kvs_port (0)
    , kvs_retry_count (2)
    // log_host
    , log_port (2438)
    , log_level ('I')
  {
    memset (socket, 0, sizeof(socket));
    memset (kvs_host, 0, sizeof (kvs_host));

    if (gethostname (log_host, MAX_HOST_LEN) != 0)
    {
      snprintf ( log_host
               , MAX_HOST_LEN
               , "localhost"
               );
    }
  }

  int  magic;
  char socket[MAX_PATH_LEN];
  char kvs_host[MAX_HOST_LEN];
  unsigned short kvs_port;
  unsigned int kvs_retry_count;
  char log_host[MAX_HOST_LEN];
  unsigned short log_port;
  char log_level;
} __attribute__((packed));

typedef gpi::api::gpi_api_t gpi_api_t;

// exit codes
static const int EX_USAGE = 2;
static const int EX_INVAL = 3;

static int configure_logging
  (const config_t *cfg, const char* logfile, boost::asio::io_service&);

namespace
{
  enum requested_api_t { API_fake, API_gaspi };
  std::unique_ptr<gpi_api_t> create_gpi_api
    ( requested_api_t requested_api
    , const unsigned long long memory_size
    , const unsigned short p
    , const std::chrono::seconds& timeout
    )
  {
    if (requested_api == API_gaspi)
    {
      return fhg::util::make_unique <gpi::api::gaspi_t> (memory_size, p, timeout);
    }
    else if (requested_api == API_fake)
    {
      return fhg::util::make_unique <gpi::api::fake_gpi_api_t> (memory_size, timeout);
    }
    else
    {
      throw std::runtime_error
        ("internal error: unhandled enum value for 'requested_api': " + std::to_string (requested_api));
    }
  }
}

int main (int ac, char *av[])
try
{
  int i = 0;
  requested_api_t requested_api = API_gaspi;
  char socket_path[MAX_PATH_LEN];
  memset (socket_path, 0, sizeof(socket_path));
  char logfile[MAX_PATH_LEN];
  memset (logfile, 0, sizeof(logfile));
  std::string default_memory_url ("gpi://?buffer_size=4194304&buffers=8");

  unsigned long long gpi_mem = (1<<26);
  unsigned int gpi_timeout = 120;
  boost::optional<unsigned short> port;

  std::vector<std::string> mem_urls;

  config_t config;

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
      fprintf(stderr, "    --socket PATH (%s)\n", socket_path);
      fprintf(stderr, "      create socket at this location\n");
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
      fprintf(stderr, "    --gpi-api STRING (%s)\n", "gaspi");
      fprintf(stderr, "      choose the GPI API to use\n");
      fprintf(stderr, "        fake - use the fake api\n");
      fprintf(stderr, "        gaspi - use the GASPI api\n");
      fprintf(stderr, "    --port PORT\n");
      fprintf(stderr, "\n");
      fprintf(stderr, "GPI options (expert)\n");
      fprintf(stderr, "    --gpi-timeout SECONDS (%u)\n", gpi_timeout);
      exit(EXIT_SUCCESS);
    }
    else if (strcmp(av[i], "--version") == 0 || strcmp(av[i], "-V") == 0)
    {
      printf("%s\n", fhg::project_version());
      exit(EXIT_SUCCESS);
    }
    else if (strcmp(av[i], "--socket") == 0)
    {
      ++i;
      if (i < ac)
      {
        if ((strlen(av[i]) + 1) > sizeof(config.socket))
        {
          fprintf(stderr, "%s: path to socket is too large!\n", program_name);
          fprintf(stderr, "    at most %lu characters are supported\n"
                 , sizeof(config.socket) - 1
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
        requested_api = strcmp (av[i], "gaspi") == 0 ? API_gaspi
          : strcmp (av[i], "fake") == 0 ? API_fake
          : throw std::runtime_error
            ("invalid argument to --gpi-api: must be 'gaspi' or 'fake'");
        ++i;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --gpi-api\n", program_name);
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
    else if (strcmp(av[i], "--port") == 0)
    {
      ++i;
      if (i < ac)
      {
        unsigned short p;
        if (sscanf(av[i], "%hu", &p) == 0)
        {
          fprintf(stderr, "%s: port invalid: %s\n", program_name, av[i]);
          exit(EX_INVAL);
        }
        ++i;
        port = p;
      }
      else
      {
        fprintf(stderr, "%s: missing argument to --gpi-timeout\n", program_name);
        exit(EX_USAGE);
      }
    }
    else if (strcmp(av[i], "--") == 0)
    {
      ++i;
      break;
    }
    else
    {
      fprintf(stderr, "%s: unknown option: %s\n", program_name, av[i]);
      ++i;
    }
  }

  if (boost::none == port)
  {
    fprintf (stderr, "parameter 'port' not given (--port <port>)\n");
    exit (EX_USAGE);
  }

  if (0 == strlen (socket_path))
  {
    fprintf (stderr, "parameter 'socket' not given (--socket <path-to-socket>)\n");
    exit (EX_USAGE);
  }

  if (0 == strlen (config.kvs_host))
  {
    fprintf (stderr, "parameter 'kvs-host' not given (--kvs-host <host-of-kvs-daemon>\n");
    exit (EX_USAGE);
  }
  if (0 == config.kvs_port)
  {
    fprintf (stderr, "parameter 'kvs-port' not given (--kvs-port <port-of-kvs-daemon>\n");
    exit (EX_USAGE);
  }

  boost::asio::io_service remote_log_io_service;
  if (0 != configure_logging (&config, logfile, remote_log_io_service))
  {
    fprintf (stderr, "could not setup logging");
  }

  snprintf ( config.socket
           , sizeof(config.socket)
           , "%s"
           , socket_path
           );

  fhg::com::kvs::kvsc_ptr_t kvs_client
    (new fhg::com::kvs::client::kvsc
      ( config.kvs_host
      , boost::lexical_cast<std::string> (config.kvs_port)
      , true
      , boost::posix_time::seconds (1)
      , config.kvs_retry_count
      )
    );

  // initialize gpi api
  std::unique_ptr<gpi_api_t> gpi_api_
    (create_gpi_api ( requested_api
                    , gpi_mem
                    , *port
                    , std::chrono::seconds (gpi_timeout)
                    )
    );
  gpi_api_t& gpi_api (*gpi_api_);

  LOG (INFO, "GPI started: " << gpi_api.rank());

  if (0 == gpi_api.rank())
  {
    LOG (INFO, "GPISpace version: " << fhg::project_version());
    LOG (INFO, "GPISpace revision: " << fhg::project_revision());
    LOG (INFO, "GPIApi version: " << gpi_api.version());
  }

  try
  {
    LOG ( TRACE
        ,  "rank=" << gpi_api.rank()
        << " dma=" << gpi_api.dma_ptr()
        << " #nodes=" << gpi_api.number_of_nodes()
        << " mem_size=" << gpi_api.memory_size()
        );

    if (mem_urls.empty ())
      mem_urls.push_back (default_memory_url);

    const gpi::pc::container::manager_t container_manager
      (config.socket, mem_urls, gpi_api, kvs_client);

    LOG (INFO, "started GPI interface on rank " << gpi_api.rank() << " at " << config.socket);

    fhg::util::thread::event<> stop_requested;
    const std::function<void()> request_stop
      (std::bind (&fhg::util::thread::event<>::notify, &stop_requested));

    fhg::util::signal_handler_manager signal_handler;
    signal_handler.add (SIGTERM, std::bind (request_stop));
    signal_handler.add (SIGINT, std::bind (request_stop));

    stop_requested.wait();

    LOG (INFO, "gpi process (rank " << gpi_api.rank() << ") terminated");
    return EXIT_SUCCESS;
  }
  catch (std::exception const & ex)
  {
    LOG (ERROR, "gpi process (rank " << gpi_api.rank() << ") failed: " << ex.what());
    return EXIT_FAILURE;
  }
}
catch (std::exception const & ex)
{
  LOG (ERROR, "GPI could not be started: " << ex.what());
  return EXIT_FAILURE;
}

static int configure_logging ( const config_t *cfg
                             , const char* logfile
                             , boost::asio::io_service& remote_log_io_service
                             )
{
  char server_url[MAX_HOST_LEN + 32];

  // logging oonfiguration
  snprintf( server_url, sizeof(server_url), "%s:%hu"
          , cfg->log_host, cfg->log_port
          );
  const char *log_level = nullptr;
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

  FHGLOG_SETUP (remote_log_io_service);

  return 0;
}
