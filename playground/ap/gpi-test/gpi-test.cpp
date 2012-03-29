#include <errno.h>
#include <GPI.h>
#include <string>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fstream>

static const char * program_name = "gpi-test";
static const float  PROGRAM_VERSION = 0.1;

static const int EX_USAGE = 2;

static const int GPI_TIMEOUT = 10;
static const int GPI_DAEMON_FAILED = 11;
static const int GPI_PORT_INUSE = 12;
static const int GPI_NET_TYPE = 13;
static const int GPI_HOST_LIST = 14;

static const int GPI_IB_FAILED = 30;
static const int GPI_IB_NO_LINK = 31;

static const int GPI_SHLIB_FAILED = 20;
static const int GPI_SHLIB_MISSING = 21;

struct config_t
{
        int  magic;
	char socket[1024];
	char kvs_host[512];
	unsigned short kvs_port;
	char log_host[512];
	unsigned short log_port;
	char log_level;
};

static void cleanupGPI()
{
  (void)killProcsGPI();
}

static void handle_keyboard_interrupt(int s)
{
  fprintf(stderr, "%s: interrupted\n", program_name);
  (void)killProcsGPI();
  exit(EXIT_FAILURE);
}

static void handle_gpi_timeout(int s)
{
  fprintf(stderr, "%s: startup timed out!\n", program_name);
  (void)killProcsGPI();
  exit(GPI_TIMEOUT);
}

int main (int ac, char **av)
{
	int i;
	std::string pidfile;
        bool daemonize;
	bool is_master;
	int verbose;

	unsigned long long gpi_mem;
	unsigned short gpi_port;
	unsigned int gpi_mtu;
	int gpi_net;
	int gpi_np;
	bool gpi_perform_checks;
	bool gpi_clear_caches;
	unsigned int gpi_timeout;

	std::ofstream ofs("/tmp/gpi-test.log");

	// default config
	daemonize = false;
	is_master = true;
	verbose = 0;
	gpi_mem = 1 << 30;
	gpi_mtu = 2048;
	gpi_net = 0;
	gpi_port = 10820;
	gpi_np = -1;
	gpi_perform_checks = true;
	gpi_clear_caches = false;
	gpi_timeout = 120;

	config_t config;
	snprintf (config.socket, sizeof(config.socket), "/tmp/S-gpi-space-%d", getuid());
	snprintf (config.kvs_host, sizeof(config.kvs_host), "localhost");
	snprintf (config.log_host, sizeof(config.log_host), "localhost");
	config.kvs_port = 2439;
	config.log_port = 2438;
	config.log_level = 'I';

	// dump command line
	for (i = 0; i < ac; ++i)
	{
		ofs << "av[" << i << "] = " << av[i] << std::endl;
	}

	// parse command line
	i = 1;
	while (i < ac)
	{
		if      (strcmp(av[i], "--help") == 0 || strcmp(av[i], "-h") == 0)
		{
			++i;
			fprintf(stderr, "%s: [options]\n", program_name);
			fprintf(stderr, "\n");
			fprintf(stderr, "      version: %0.2f\n", PROGRAM_VERSION);
			fprintf(stderr, "      GPI version: %0.2f\n", getVersionGPI());
			fprintf(stderr, "\n");
			fprintf(stderr, "options\n");
			fprintf(stderr, "    --help|-h\n");
			fprintf(stderr, "      print this help information\n");
			fprintf(stderr, "    --version|-V\n");
			fprintf(stderr, "      print version information\n");
			fprintf(stderr, "\n");
			fprintf(stderr, "    --verbose|-v\n");
			fprintf(stderr, "      be verbose\n");
			fprintf(stderr, "\n");
			fprintf(stderr, "    --pidfile PATH\n");
			fprintf(stderr, "      write master's PID to this file\n");
			fprintf(stderr, "\n");
			fprintf(stderr, "    --daemonize\n");
			fprintf(stderr, "      fork to background when all checks were ok\n");
			fprintf(stderr, "\n");
			fprintf(stderr, "    --socket PATH (%s)\n", config.socket);
			fprintf(stderr, "      listen for process containers on this path\n");
			fprintf(stderr, "\n");
			fprintf(stderr, "KVS options\n");
			fprintf(stderr, "    --kvs-host HOST (%s)\n", config.kvs_host);
			fprintf(stderr, "    --kvs-port PORT (%hu)\n", config.kvs_port);
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
			fprintf(stderr, "    --gpi-no-checks\n");
			fprintf(stderr, "      do not perform checks before gpi startup\n");
			fprintf(stderr, "    --gpi-clear-caches\n");
			fprintf(stderr, "      clear file caches on all nodes\n");
			exit(EXIT_SUCCESS);
		}
		else if (strcmp(av[i], "--version") == 0 || strcmp(av[i], "-V") == 0)
		{
			printf("GPI %0.2f %0.2f\n", getVersionGPI(), PROGRAM_VERSION);
			exit(EXIT_SUCCESS);
		}
		else if (strcmp(av[i], "--pidfile") == 0)
		{
			++i;
			if (i < ac)
			{
			  pidfile = av[i];
			  ++i;
			}
			else
			{
				fprintf(stderr, "%s: missing argument to --pidfile\n", program_name);
				exit(EX_USAGE);
			}
		}
		else if (strcmp(av[i], "--verbose") == 0 || strcmp(av[i], "-v") == 0)
		{
			++i;
			++verbose;
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
			  snprintf(config.socket, sizeof(config.socket), "%s", av[i]);
			  ++i;
			}
			else
			{
				fprintf(stderr, "%s: missing argument to --socket\n", program_name);
				exit(EX_USAGE);
			}
		}
		else if (strcmp(av[i], "--kvs-host") == 0)
		{
			++i;
			if (i < ac)
			{
			  snprintf(config.kvs_host, sizeof(config.kvs_host), "%s", av[i]);
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
					exit(EX_USAGE);
				}
				++i;
			}
			else
			{
				fprintf(stderr, "%s: missing argument to --kvs-port\n", program_name);
				exit(EX_USAGE);
			}
		}
		else if (strcmp(av[i], "--log-host") == 0)
		{
			++i;
			if (i < ac)
			{
			  snprintf(config.log_host, sizeof(config.log_host), "%s", av[i]);
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
					exit(EX_USAGE);
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
					exit(EX_USAGE);
				}
				++i;
			}
			else
			{
				fprintf(stderr, "%s: missing argument to --gpi-mem\n", program_name);
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
					exit(EX_USAGE);
				}
				++i;
				ofs << "set port to " << gpi_port << std::endl;
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
					exit(EX_USAGE);
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
					exit(EX_USAGE);
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
					exit(EX_USAGE);
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
					exit(EX_USAGE);
				}
				++i;
			}
			else
			{
				fprintf(stderr, "%s: missing argument to --gpi-np\n", program_name);
				exit(EX_USAGE);
			}
		}
		else if (strcmp(av[i], "--gpi-no-checks") == 0)
		{
			++i;
			gpi_perform_checks = false;
		}
		else if (strcmp(av[i], "--gpi-clear-caches") == 0)
		{
			++i;
			gpi_clear_caches = true;
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
					exit(EX_USAGE);
				}
			}
		}
		else
		{
			fprintf(stderr, "%s: unknown option: %s\n", program_name, av[i]);
			++i;
		}
	}

	ofs << "is-master = " << is_master << std::endl;

	ofs << "port = " << gpi_port << std::endl;

	if (setPortGPI(gpi_port) != 0)
	{
		fprintf(stderr, "%s: could not set port to %hu\n", program_name, gpi_port);
		exit (GPI_PORT_INUSE);
	}

	if (is_master)
	{
		// AP: do those calls actually work???  I could not find out a
		// way how to get this working, the information is not
		// transported to the workers, is it?
		if (setNetworkGPI((GPI_NETWORK_TYPE)gpi_net) != 0)
		{
			fprintf(stderr, "%s: could not set network type to %d\n", program_name, gpi_net);
			exit (EXIT_FAILURE);
		}
		if (setMtuSizeGPI(gpi_mtu) != 0)
		{
			fprintf(stderr, "%s: could not set MTU to %u\n", program_name, gpi_mtu);
			exit (EXIT_FAILURE);
		}
		setNpGPI(gpi_np);

		// perform GPI checks
		int number_of_nodes = generateHostlistGPI();

		if (number_of_nodes <= 0)
		{
			fprintf(stderr, "%s: could not generate hostlist: %d\n", program_name, number_of_nodes);
			exit (GPI_HOST_LIST);
		}

		if (gpi_perform_checks)
		{
		  fprintf(stderr, "performing GPI checks...\n");
		  for (i = 0 ; i < number_of_nodes ; ++i)
		  {
		  	const char * hostname = getHostnameGPI(i);
			int error_code;

			fprintf(stderr, "%s...", hostname);

			if (verbose) fprintf(stderr, "\n");
			
			error_code = checkPortGPI(hostname, getPortGPI());
			if (error_code == 0)
			{
			  if (verbose) fprintf(stderr, "  * %s port %hu: ok\n", hostname, getPortGPI());
			}
			else if (error_code == -42)
			{
			  fprintf(stderr, "*** %s port %hu: timeout\n", hostname, getPortGPI());
			  exit(GPI_TIMEOUT);
			}
			else
			{
			  fprintf(stderr, "*** %s port %hu: in use\n", hostname, getPortGPI());
			  exit(GPI_PORT_INUSE);
			}

			error_code = pingDaemonGPI(hostname);
			if (error_code == 0)
			{
			   if (verbose) fprintf(stderr, "  * %s daemon: ok\n", hostname);
			}
			else
			{
			   fprintf(stderr, "*** %s daemon: failed\n", hostname);
			   exit(GPI_DAEMON_FAILED);
			}

			error_code = checkSharedLibsGPI(hostname);
			if (error_code == 0)
			{
			   if (verbose) fprintf(stderr, "  * %s shared libs: ok\n", hostname);
			}
			else if (error_code == -1)
			{
			   fprintf(stderr, "*** %s shared libs: failed\n", hostname);
			   exit(GPI_SHLIB_FAILED);
			}
			else if (error_code == -2)
			{
			   fprintf(stderr, "*** %s shared libs: missing\n", hostname);
			   exit(GPI_SHLIB_MISSING);
			}
			else if (error_code == -42)
			{
			   fprintf(stderr, "*** %s shared libs: timeout\n", hostname);
			   exit(GPI_TIMEOUT);
			}

			error_code = runIBTestGPI(hostname);
			if (error_code == 0)
			{
			   if (verbose) fprintf(stderr, "  * %s IB test: ok\n", hostname);
			}
			else if (error_code == -2)
			{
			   fprintf(stderr, "*** %s IB test: no link\n", hostname);
			   exit(GPI_IB_NO_LINK);
			}
			else if (error_code == -42)
			{
			   fprintf(stderr, "*** %s IB test: timeout\n", hostname);
			   exit(GPI_TIMEOUT);
			}
			else
			{
			   fprintf(stderr, "*** %s IB test: failed\n", hostname);
			   exit(GPI_IB_FAILED);
			}

			fprintf(stderr, "OK\n", hostname);
		  }
		  fprintf(stderr, "all checks OK!\n");
		}

		if (gpi_clear_caches)
		{
		  fprintf(stderr, "clearing caches...\n", program_name);
		  for (i = 0 ; i < number_of_nodes ; ++i)
		  {
		    const char * hostname = getHostnameGPI(i);
		    int error_code = clearFileCacheGPI(hostname);
		    if (1 == error_code)
		    {
		      fprintf(stderr, "    %s: ok\n", hostname);
		    }
		    else if (-42 == error_code)
		    {
		      fprintf(stderr, "    %s: timeout\n", hostname);
		    }
		    else
		    {
		      fprintf(stderr, "    %s: failed\n", hostname);
		    }
		  }
		}

		std::ofstream pidfile_stream (pidfile.c_str());
		if (not pidfile.empty())
		{
			if (not pidfile_stream.good())
			{
				fprintf(stderr, "%s: could not open pidfile: %s: %s\n", program_name, pidfile.c_str(), strerror(errno));
				exit(EXIT_FAILURE);
			}
		}

		// everything is fine so far, daemonize
		if (daemonize)
		{
			if (pid_t child = fork())
			{
				if (child == -1)
				{
					fprintf(stderr, "%s: could not fork: %s\n", program_name, strerror(errno));
					exit (EXIT_FAILURE);
				}
				else
				{
					fprintf(stderr, "daemon pid %d\n", child);
					fflush(stderr);
					exit (EXIT_SUCCESS);
				}
			}
			setsid();
			close(0); close(1); close(2);
		}

		if (not pidfile.empty())
		{
		  pidfile_stream << getpid() << std::flush << std::endl;
		}
	}

	int gpi_argc = ac;
	if (is_master)
	{
	  gpi_argc = 1;
          signal(SIGALRM, handle_gpi_timeout);
          signal(SIGINT, handle_keyboard_interrupt);
          alarm(gpi_timeout);
	}

	if (startGPI (gpi_argc, av, av[0], gpi_mem) != 0)
	{
	  ofs << "failed to start GPI!" << std::endl;
	  return EXIT_FAILURE;
	}

	ofs << "started gpi: " << getRankGPI() << std::endl;

	if (is_master)
	{
                alarm(0);
                config.magic = 0xdeadbeef;
		memcpy(getDmaMemPtrGPI(), &config, sizeof(config));
		size_t max_enqueued_requests = getQueueDepthGPI();
		for (size_t rank = 1 ; rank < getNodeCountGPI() ; ++rank)
		{
			ofs << "distributing config structure to rank " << rank << std::endl;
			if (openDmaPassiveRequestsGPI() >= max_enqueued_requests)
			{
				if (-1 == waitDmaPassiveGPI())
				{
				   ofs << program_name << ": could not wait on passive channel" << std::flush << std::endl;
				   killProcsGPI();
				   exit(EXIT_FAILURE);
				}
			}
			
			if (-1 == sendDmaPassiveGPI ( 0 // local_offset
		                                    , sizeof(config)
                                                    , rank
                                                    ))
                        {
                           ofs << program_name << ": could not send config to " << rank << std::flush << std::endl;
                           killProcsGPI();
                           exit(EXIT_FAILURE);
                        }
		}
		if (-1 == waitDmaPassiveGPI())
		{
		   ofs << program_name << ": could not wait on passive channel" << std::flush << std::endl;
		   fprintf(stderr, "%s: could not wait on passive channel\n", program_name);
		   killProcsGPI();
		   exit(EXIT_FAILURE);
		}
	}
	else
	{
		ofs << "receiving config..." << std::endl;
		int source = 0;
		recvDmaPassiveGPI ( 0
                                  , sizeof(config)
                                  , &source
                                  );
		ofs << "got config from " << source << std::endl;
		memcpy(&config, getDmaMemPtrGPI(), sizeof(config));
		if (config.magic != 0xdeadbeef)
		{
			ofs << "magic is invalid!!!!!" << std::endl;
		}
		else
		{
			ofs << "configuring..." << std::endl;
			gpi_mem = getMemWorkerGPI();
			gpi_port = getPortGPI();
		}
	}

	ofs << "config.kvs_host = " << config.kvs_host << std::endl;
	ofs << "config.kvs_port = " << config.kvs_port << std::endl;
	ofs << "config.log_host = " << config.log_host << std::endl;
	ofs << "config.log_port = " << config.log_port << std::endl;
	ofs << "config.socket = " << config.socket << std::endl;
	ofs << "worker mem = " << getMemWorkerGPI() << std::endl;

	barrierGPI();

	shutdownGPI();

	return 0;
}
