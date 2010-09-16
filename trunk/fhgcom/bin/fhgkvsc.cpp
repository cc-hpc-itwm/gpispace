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

  std::string key;
  std::string value;

  std::vector<std::string> key_list;

  po::options_description desc ("options");
  desc.add_options()
    ("help,h", "print this help")
    ("host,H", po::value<std::string>(&server_address)->default_value(server_address), "use this host")
    ("port,P", po::value<std::string>(&server_port)->default_value(server_port), "port or service name to use")
    ("key,k", po::value<std::string>(&key), "key to put or get")
    ("value,v", po::value<std::string>(&value), "value to store")
    ("full,f", "key must match completely")

    ("save,s", "save the database on the server")
    ("load,l", "reload the database on the server")
    ("list,L", "list entries in the server")
    ("clear,C", "clear entries on the server")

    ("put,p", po::value<std::string>(&key), "store a value in the key-value store")
    ("get,g", po::value<std::vector<std::string> >(&key_list), "get values from the key-value store")
    ("del,d", po::value<std::vector<std::string> >(&key_list), "delete entries from the key-value store")
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

  fhg::com::kvs::client::kvsc client;

  client.start (server_address, server_port);

  if (vm.count ("load"))
  {
    client.load();
  }
  else if (vm.count ("save"))
  {
    client.save();
  }
  else if (vm.count ("list"))
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
      return 1;
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
      return 1;
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
      return count > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
  }
  else if (vm.count("put"))
  {
    if (value.empty())
    {
      throw std::runtime_error ("put: value must not be empty");
    }

    try
    {
      client.put (key, value);
    }
    catch (std::exception const & ex)
    {
      std::cerr << "E: " << ex.what() << std::endl;
      return 1;
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
    return count > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  client.stop();
  return 0;
}
