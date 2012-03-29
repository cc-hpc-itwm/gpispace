#include <errno.h>
#include <GPI/GPI.h>
#include <string>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>

static const char * program_name = "gpi-test";

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

int main (int ac, char **av)
{
	int i;
	std::string pidfile;
        bool daemonize;

	unsigned long long gpi_mem;
	unsigned short gpi_port;
	unsigned int gpi_mtu;
	int gpi_net;
	unsigned int gpi_np;
	bool gpi_perform_checks;
	bool gpi_clear_caches;

	std::ofstream ofs("/tmp/gpi-test.log");

	// default config
	daemonize = false;
	gpi_mem = 1 << 30;
	gpi_mtu = 2048;
	gpi_net = 0;
	gpi_port = 10820;
	gpi_np = 0;
	gpi_perform_checks = true;
	gpi_clear_caches = false;

	config_t config;
	sprintf (config.socket, "");
	sprintf (config.kvs_host, "");
	sprintf (config.log_host, "");
	config.kvs_port = 2439;
	config.log_port = 0;
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
			fprintf(stderr, "%s: -h|--help options\n", program_name);
			fprintf(stderr, "\n");
			fprintf(stderr, "    --pidfile PATH\n");
			fprintf(stderr, "    --daemonize\n");
			fprintf(stderr, "    --socket PATH\n");
			fprintf(stderr, "    --kvs-host HOST\n");
			fprintf(stderr, "    --kvs-port PORT (%hu)\n", config.kvs_port);
			fprintf(stderr, "\n");
			fprintf(stderr, " LOG options\n");
			fprintf(stderr, "    --log-host HOST\n");
			fprintf(stderr, "    --log-port PORT\n");
			fprintf(stderr, "    --log-level {T, D, I, W, E, F}\n");
			fprintf(stderr, "\n");
			fprintf(stderr, " GPI options\n");
			fprintf(stderr, "    --gpi-mem SIZE (%llu)\n", gpi_mem);
			fprintf(stderr, "\n");
			fprintf(stderr, " GPI options (expert)\n");
			fprintf(stderr, "    --gpi-port PORT (%hu)\n", gpi_port);
			fprintf(stderr, "    --gpi-mtu SIZE (%u)\n", gpi_mtu);
			fprintf(stderr, "    --gpi-net TYPE (%d)\n", gpi_net);
			fprintf(stderr, "              0=IB\n");
			fprintf(stderr, "              1=ETH\n");
			fprintf(stderr, "    --gpi-no-checks\n");
			fprintf(stderr, "      do not perform checks before gpi startup\n");
			fprintf(stderr, "    --gpi-clear-caches\n");
			fprintf(stderr, "      clear file caches on all nodes\n");
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
		else if (strcmp(av[i], "--gpi-mem") == 0)
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
		else if (strcmp(av[i], "--gpi-port") == 0)
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
		else if (strcmp(av[i], "--gpi-np") == 0)
		{
			++i;
			if (i < ac)
			{
				if (sscanf(av[i], "%u", &gpi_np) == 0)
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
		else
		{
			break;
		}
	}
	char **gpi_argv = av+(i-1);
	int    gpi_argc = ac-(i-1);

	// dump gpi_command command line
	for (i = 0; i < gpi_argc; ++i)
	{
		ofs << "gpi_av[" << i << "] = " << gpi_argv[i] << std::endl;
	}

	int is_master = isMasterProcGPI(gpi_argc, gpi_argv);

	ofs << "is-master = " << is_master << std::endl;

	// work around bug in startGPI, so that we use the same port as the master...
	if (not is_master)
	{
		i = 1;
		while (i < ac)
		{
		  if (strcmp(av[i], "-p") == 0)
		  {
			++i;
			if (i < ac)
			{
				if (sscanf(av[i], "%hu", &gpi_port) == 0)
				{
					ofs << "gpi-port invalid: " << av[i] << std::endl;
					exit(EX_USAGE);
				}
				++i;
			}
			else
			{
				ofs << "missing argument to -p" << std::endl;
				exit(EX_USAGE);
			}
		}
		else
		{
			++i;
		}
	      }
	}

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
			
			error_code = checkPortGPI(hostname, getPortGPI());
			if (error_code == 0)
			{
			  fprintf(stderr, "  * %s port %hu: ok\n", hostname, getPortGPI());
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
			   fprintf(stderr, "  * %s daemon: ok\n", hostname);
			}
			else
			{
			   fprintf(stderr, "*** %s daemon: failed\n", hostname);
			   exit(GPI_DAEMON_FAILED);
			}

			error_code = checkSharedLibsGPI(hostname);
			if (error_code == 0)
			{
			   fprintf(stderr, "  * %s shared libs: ok\n", hostname);
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
			   fprintf(stderr, "  * %s IB test: ok\n", hostname);
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
				fprintf(stderr, "daemon pid %d\n", child);
				fflush(stderr);
				exit (EXIT_SUCCESS);
			}
			setsid();
			close(0); close(1); close(2);
		}

		if (not pidfile.empty())
		{
		  pidfile_stream << getpid() << std::endl;
		}
	}

	if (startGPI (gpi_argc, gpi_argv, av[0], gpi_mem) != 0)
	{
		ofs << "failed to start GPI!" << std::endl;
		return EXIT_FAILURE;
	}
	ofs << "started gpi: " << getRankGPI() << std::endl;

	if (is_master)
	{
                config.magic = 0xdeadbeef;
		memcpy(getDmaMemPtrGPI(), &config, sizeof(config));
		size_t max_enqueued_requests = getQueueDepthGPI();
		for (size_t rank = 1 ; rank < getNodeCountGPI() ; ++rank)
		{
			ofs << "distributing config structure to rank " << rank << std::endl;
			if (openDMAPassiveRequestsGPI() >= max_enqueued_requests)
			{
				waitDmaPassiveGPI();
			}
			
			sendDmaPassiveGPI ( 0 // local_offset
		                          , sizeof(config)
                                          , rank
                                          );
		}
		waitDmaPassiveGPI();
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
