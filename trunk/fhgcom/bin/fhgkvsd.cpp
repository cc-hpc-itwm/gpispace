#include <fhglog/fhglog.hpp>

#include <stdlib.h>
#include <csignal>

#include <iostream>
#include <boost/program_options.hpp>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/peer_info.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>

static fhg::com::io_service_pool pool (1);
static fhg::com::kvs::server::kvsd *g_kvsd (0);

void save_state (int s)
{
  try
  {
    LOG(DEBUG, "saving state due to signal: " << s);
    g_kvsd->save();
  }
  catch (std::exception const & ex)
  {
    LOG(WARN, "could not persist state: " << ex.what());
  }
}
void load_state (int s)
{
  try
  {
    LOG(DEBUG, "loading state due to signal: " << s);
    g_kvsd->load();
  }
  catch (std::exception const & ex)
  {
    LOG(WARN, "could not load state: " << ex.what());
  }
}
void clear_state (int s)
{
  try
  {
    LOG(DEBUG, "clearing state due to signal: " << s);
    g_kvsd->clear("");
  }
  catch (std::exception const & ex)
  {
    LOG(WARN, "could not clear state: " << ex.what());
  }
}

void signal_handler (int s)
{
  pool.stop();
  save_state (s);
}

int main(int ac, char *av[])
{
  FHGLOG_SETUP(ac,av);

  namespace po = boost::program_options;

  std::string server_address ("*");
  std::string server_port ("2439");

  if (getenv("KVS_URL") != NULL)
  {
    try
    {
      using namespace fhg::com;
      peer_info_t pi (peer_info_t::from_string (getenv("KVS_URL")));
      server_address = pi.host(server_address);
      server_port = pi.port(server_port);
    }
    catch (std::exception const & ex)
    {
      std::cerr << "W: malformed environment variable KVS_URL: " << ex.what() << std::endl;
    }
  }

  bool reuse_address (true);
  std::string home (getenv("HOME") ? getenv("HOME") : ".");
  std::string default_store_path(home + "/.fhgkvsd.dat");
  std::string store_path (getenv("KVS_STORE") ? getenv("KVS_STORE") : default_store_path);

  po::options_description desc ("options");
  desc.add_options()
    ("help,h", "print this help")
    ("bind,b", po::value<std::string>(&server_address)->default_value(server_address), "bind to this address")
    ("port,p", po::value<std::string>(&server_port)->default_value(server_port), "port or service name to use")
    ("reuse-address", po::value<bool>(&reuse_address)->default_value(reuse_address), "reuse address")
    ("store,s", po::value<std::string>(&store_path)->default_value(store_path), "path to persistent store, set KVS_STORE to override default")
    ("clear,C", "start with an empty store")
    ;

  po::variables_map vm;
  try
  {
    po::store (po::parse_command_line (ac, av, desc), vm);
    po::notify (vm);
  }
  catch (std::exception const & ex)
  {
    std::cerr << "invalid argument: " << ex.what() << std::endl;
    std::cerr << "try " << av[0] << " -h to get some help" << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count ("help"))
  {
    std::cerr << "usage: " << av[0] << std::endl;
    std::cerr << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_SUCCESS;
  }

  fhg::com::kvs::server::kvsd kvsd (store_path);
  g_kvsd = &kvsd;

  if (vm.count ("clear")) kvsd.clear ("");

  if (server_address == "*")
  {
    server_address = "0";
  }

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGUSR1, save_state);
  signal(SIGUSR2, clear_state);
  signal(SIGHUP, load_state);


  try
  {
    fhg::com::tcp_server server ( pool
                                , kvsd
                                , server_address
                                , server_port
                                , reuse_address
                                );

    server.start ();
    pool.run();
  }
  catch (std::exception const & ex)
  {
    std::cerr << "could not start server: " << ex.what () << std::endl;
    return EXIT_FAILURE;
  }

  return 0;
}
