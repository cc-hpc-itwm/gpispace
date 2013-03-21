#include <fhglog/fhglog.hpp>

#include <iostream>
#include <boost/program_options.hpp>

#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/peer_info.hpp>
#include <fhgcom/kvs/kvsc.hpp>

enum my_exit_codes
  {
     EX_OK = EXIT_SUCCESS
   , EX_ERR = EXIT_FAILURE
     , EX_INVAL // invalid argument
     , EX_SEARCH // key not found
     , EX_CONN // connection failed
  };


int main(int ac, char *av[])
{
  FHGLOG_SETUP(ac,av);

  namespace po = boost::program_options;

  std::string server_address ("localhost");
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

  std::string key;
  std::string value;
  size_t expiry (0);
  size_t timeout (120 * 1000);

  std::vector<std::string> key_list;

  po::options_description desc ("options");
  desc.add_options()
    ("help,h", "print this help")
    ("host,H", po::value<std::string>(&server_address)->default_value(server_address), "use this host")
    ("port,P", po::value<std::string>(&server_port)->default_value(server_port), "port or service name to use")
    ("key,k", po::value<std::string>(&key), "key to put or get")
    ("value,v", po::value<std::string>(&value), "value to store")
    ("expiry,e", po::value<size_t>(&expiry)->default_value (expiry), "expiry of entry in milli seconds")
    ("full,f", "key must match completely")

    ("save,S", "save the database on the server")
    ("load,L", "reload the database on the server")
    ("list-all,l", "list all entries in the server")
    ("clear,C", "clear entries on the server")
    ("term", "terminate a running kvs daemon")
    ("timeout,T", po::value<size_t>(&timeout)->default_value (timeout), "timeout in milliseconds")
    ("put,p", po::value<std::string>(&key), "store a value in the key-value store")
    ("get,g", po::value<std::vector<std::string> >(&key_list), "get values from the key-value store")
    ("del,d", po::value<std::vector<std::string> >(&key_list), "delete entries from the key-value store")
    ("cnt", po::value<std::string>(&key), "atomically increment/decrement a numeric entry")
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
    return EX_INVAL;
  }

  if (vm.count ("help"))
  {
    std::cerr << "usage: " << av[0] << std::endl;
    std::cerr << std::endl;
    std::cerr << desc << std::endl;
    return EX_OK;
  }

  fhg::com::kvs::client::kvsc client;

  try
  {
    client.start ( server_address
                 , server_port
                 , true
                 , boost::posix_time::milliseconds(timeout)
                 , 1
                 );
  }
  catch (std::exception const & ex)
  {
    std::cerr << "E: " << ex.what() << std::endl;
    return EX_CONN;
  }

  if (vm.count ("load"))
  {
    try
    {
      client.load();
    }
    catch (std::exception const & ex)
    {
      std::cerr << "E: " << ex.what() << std::endl;
      return EX_CONN;
    }
  }
  else if (vm.count ("save"))
  {
    try
    {
      client.save();
    }
    catch (std::exception const & ex)
    {
      std::cerr << "E: " << ex.what() << std::endl;
      return EX_CONN;
    }
  }
  else if (vm.count ("term"))
  {
    try
    {
      client.term (15, "client requested termination");
    }
    catch (std::exception const &ex)
    {
      std::cerr << "E: " << ex.what() << std::endl;
      return EX_CONN;
    }
  }
  else if (vm.count ("list-all"))
  {
    try
    {
      std::map<std::string, std::string> entries (client.list());
      for ( std::map<std::string, std::string>::const_iterator e (entries.begin())
          ; e != entries.end()
          ; ++e
        )
      {
        std::cout << e->first << " = " << e->second << std::endl;
      }
    }
    catch (std::exception const & ex)
    {
      std::cerr << "E: " << ex.what() << std::endl;
      return EX_CONN;
    }
  }
  else if (vm.count ("clear"))
  {
    try
    {
      client.clear();
    }
    catch (std::exception const & ex)
    {
      std::cerr << "E: " << ex.what() << std::endl;
      return EX_CONN;
    }
  }
  else if (vm.count ("get"))
  {
    std::size_t count (0);

    for ( std::vector<std::string>::const_iterator k (key_list.begin())
        ; k != key_list.end()
        ; ++k
        )
    {
      try
      {
        std::map<std::string, std::string> entries (client.get(*k));
        if (vm.count("full"))
        {
          if (entries.find(*k) != entries.end())
          {
            std::cout << entries.at(*k) << std::endl;
            count = 1;
          }
          else if (! value.empty())
          {
            std::cout << value << std::endl;
          }
          else
          {
            throw std::runtime_error ("no such entry: " + *k);
          }
        }
        else
        {
          for ( std::map<std::string, std::string>::const_iterator e (entries.begin())
              ; e != entries.end()
              ; ++e
              )
          {
            std::cout << e->first << " = " << e->second << std::endl;
            ++count;
          }
        }
      }
      catch (std::exception const & ex)
      {
        std::cerr << "E: " << ex.what() << std::endl;
      }
    }
    return count > 0 ? EX_OK : EX_ERR;
  }
  else if (vm.count("put"))
  {
    if (value.empty())
    {
      std::cerr << "E: put: value must not be empty" << std::endl;
      return EX_INVAL;
    }

    try
    {
      if (expiry)
        client.timed_put (key, value, expiry);
      else
        client.put (key, value);
    }
    catch (std::exception const & ex)
    {
      std::cerr << "E: " << ex.what() << std::endl;
      return EX_CONN;
    }
  }
  else if (vm.count("cnt"))
  {
    int step = 1;
    if (! value.empty())
    {
      try
      {
        step = boost::lexical_cast<int>(value);
      }
      catch (std::exception const &)
      {
        std::cerr << "invalid argument: value must be an integer: " << value << std::endl;
        return EX_INVAL;
      }
    }

    try
    {
      std::cout << client.inc (key, step) << std::endl;
    }
    catch (std::exception const & ex)
    {
      std::cerr << "E: cnt operation failed: " << ex.what() << std::endl;
      return EX_CONN;
    }
  }
  else if (vm.count ("del"))
  {
    std::size_t count (0);
    for ( std::vector<std::string>::const_iterator k (key_list.begin())
        ; k != key_list.end()
        ; ++k
        )
    {
      try
      {
        client.del (*k);
        ++count;
      }
      catch (std::exception const & ex)
      {
        std::cerr << "E: " << ex.what() << std::endl;
      }
    }
    return count > 0 ? EX_OK : EX_ERR;
  }

  client.stop();
  return EX_OK;
}
