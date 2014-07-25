#include <fhglog/LogMacros.hpp>

#include <iostream>
#include <boost/program_options.hpp>

#include <fhgcom/peer_info.hpp>
#include <fhgcom/kvs/kvsc.hpp>

enum my_exit_codes
  {
     EX_OK = EXIT_SUCCESS
   , EX_ERR = EXIT_FAILURE
     , EX_INVAL // invalid argument
  };


int main(int ac, char *av[])
{
  FHGLOG_SETUP();

  namespace po = boost::program_options;

  size_t timeout (120 * 1000);

  std::vector<std::string> key_list;

  po::options_description desc ("options");
  desc.add_options()
    ("host", po::value<std::string>()->required(), "use this host")
    ("port", po::value<std::string>()->required(), "port or service name to use")
    ("get,g", po::value<std::vector<std::string>>(&key_list)->required(), "get values from the key-value store")
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
    std::cerr << "usage: " << desc << std::endl;
    return EX_INVAL;
  }

  fhg::com::kvs::client::kvsc client ( vm["host"].as<std::string>()
                                     , vm["port"].as<std::string>()
                                     , true
                                     , boost::posix_time::milliseconds(timeout)
                                     , 1
                                     );

    std::size_t count (0);

    for ( std::vector<std::string>::const_iterator k (key_list.begin())
        ; k != key_list.end()
        ; ++k
        )
    {
      try
      {
        std::map<std::string, std::string> entries (client.get(*k));
          for ( std::map<std::string, std::string>::const_iterator e (entries.begin())
              ; e != entries.end()
              ; ++e
              )
          {
            std::cout << e->first << " = " << e->second << std::endl;
            ++count;
          }
      }
      catch (std::exception const & ex)
      {
        std::cerr << "E: " << ex.what() << std::endl;
      }
    }
    return count > 0 ? EX_OK : EX_ERR;
}
