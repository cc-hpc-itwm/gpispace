#pragma once

#include <fhglog/Logger.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/optional.hpp>

#include <string>

namespace fhg
{
  namespace log
  {
    // - to_console unset
    // - remainder from arguments
    void configure
      ( Logger& logger
      , boost::asio::io_service& remote_log_io_service
      , std::string level
      , boost::optional<char const*> to_file
      , boost::optional<char const*> to_server // ip:port
      );

    // - level = trace
    // - to_console = stderr
    // - remainder unset
    void configure_to_stderr (Logger&);
  }
}
