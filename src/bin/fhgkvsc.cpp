#include <fhgcom/header.hpp>
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
  boost::asio::io_service remote_log_io_service;
  FHGLOG_SETUP (remote_log_io_service);

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
    ( "name"
    , boost::program_options::value<std::string>()->required()
    , "get host and port for name"
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
    boost::asio::io_service io_service;
    fhg::com::kvs::client::kvsc client ( io_service
                                       , vm["host"].as<std::string>()
                                       , vm["port"].as<std::string>()
                                       , true
                                       , boost::posix_time::seconds (120)
                                       , 1
                                       );

    fhg::com::p2p::address_t const address (vm["name"].as<std::string>());

    std::string const prefix ("p2p.peer." + fhg::com::p2p::to_string (address));
    fhg::com::kvs::values_type const peer_info (client.get (prefix));

    std::cout << peer_info.at (prefix + ".location.host") << "\n";
    std::cout << peer_info.at (prefix + ".location.port") << "\n";

    return EX_OK;
  }
  catch (std::exception const& ex)
  {
    std::cerr << "E: " << ex.what() << std::endl;

    return EX_ERR;
  }
}
