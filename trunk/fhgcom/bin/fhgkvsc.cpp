#include <fhglog/fhglog.hpp>

#include <iostream>
#include <boost/program_options.hpp>

#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/kvs/kvsc.hpp>

int main(int ac, char *av[])
{
  FHGLOG_SETUP(ac,av);

  namespace po = boost::program_options;

  std::string server_address ("");
  std::string server_port ("2439");
  std::string mode ("get");
  std::string key;
  std::string value;

  po::options_description desc ("options");
  desc.add_options()
    ("help,h", "print this help")
    ("server,s", po::value<std::string>(&server_address)->default_value(server_address), "use this server")
    ("port,P", po::value<std::string>(&server_port)->default_value(server_port), "port or service name to use")
    ("mode,m", po::value<std::string>(&mode)->default_value(mode), "mode can be one of put or get")
    ("key,k", po::value<std::string>(&key), "key to put or get")
    ("value,v", po::value<std::string>(&value), "value to store")
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

  fhg::com::io_service_pool pool (1);
  boost::thread thrd (boost::bind ( &fhg::com::io_service_pool::run
                                  , &pool
                                  )
                     );

  fhg::com::kvs::client::kvsc client ( pool.get_io_service()
                                     , server_address
                                     , server_port
                                     );

  if (mode == "get")
  {
    if (key.empty())
    {
      throw std::runtime_error ("mode: get: key must not be empty");
    }

    std::cout << client.get (key) << std::endl;
  }
  else if (mode == "put")
  {
    if (key.empty())
    {
      throw std::runtime_error ("mode: put: key must not be empty");
    }

    if (value.empty())
    {
      throw std::runtime_error ("mode: put: value must not be empty");
    }

    client.put (key, value);
  }
  else if (mode == "del")
  {
    if (key.empty())
    {
      throw std::runtime_error ("mode: del: key must not be empty");
    }

    client.del (key);
  }
  else if (mode == "save")
  {
    client.save();
  }
  else if (mode == "load")
  {
    client.load();
  }
  else
  {
    throw std::runtime_error ("invalid mode value: " + mode);
  }

  pool.stop();
  return 0;
}
