#include <fhgcom/kvs/kvsc.hpp>

#include <fhglog/LogMacros.hpp>

#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/program_options.hpp>

#include <iostream>

namespace
{
  enum
  { EX_OK = EXIT_SUCCESS
  , EX_ERR = EXIT_FAILURE
  , EX_INVAL // invalid argument
  };
}

int main (int argc, char *argv[])
{
  FHGLOG_SETUP();

  boost::program_options::options_description desc ("fhgkvsc options");

  desc.add_options()
    ( "host"
    , boost::program_options::value<std::string>()->required()
    , "use this host"
    )
    ( "port"
    , boost::program_options::value<std::string>()->required()
    , "port or service name to use"
    )
    ( "get,g"
    , boost::program_options::value<std::vector<std::string>>()->required()
    , "get values from the key-value store"
    );

  boost::program_options::variables_map vm;

  try
  {
    boost::program_options::store
      (boost::program_options::parse_command_line (argc, argv, desc), vm);
    boost::program_options::notify (vm);
  }
  catch (std::exception const& ex)
  {
    std::cerr << "invalid argument: " << ex.what() << std::endl;
    std::cerr << "usage: " << desc << std::endl;

    return EX_INVAL;
  }

  try
  {
    fhg::com::kvs::client::kvsc client ( vm["host"].as<std::string>()
                                       , vm["port"].as<std::string>()
                                       , true
                                       , boost::posix_time::seconds (120)
                                       , 1
                                       );

    std::size_t count (0);

    for (std::string const& k : vm["get"].as<std::vector<std::string>>())
    {
      for ( std::pair<std::string, std::string> const& key_value
          : client.get (k)
          )
      {
        std::cout << key_value.first << " = " << key_value.second << std::endl;

        ++count;
      }
    }

    return (count > 0) ? EX_OK : EX_ERR;
  }
  catch (std::exception const& ex)
  {
    std::cerr << "E: " << ex.what() << std::endl;

    return EX_ERR;
  }
}
