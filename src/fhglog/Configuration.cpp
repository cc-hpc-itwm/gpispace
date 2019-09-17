#include <fhglog/Configuration.hpp>

#include <fhglog/appender/file.hpp>
#include <fhglog/appender/stream.hpp>
#include <fhglog/format.hpp>
#include <fhglog/level.hpp>
#include <fhglog/remote/appender.hpp>

namespace fhg
{
  namespace log
  {
    void configure
      ( Logger& logger
      , boost::asio::io_service& remote_log_io_service
      , std::string level
      , boost::optional<char const*> const& to_file
      , boost::optional<char const*> const& to_server
      )
    {
      if (to_file)
      {
        logger.addAppender<FileAppender>
          (*to_file, default_format::SHORT());
      }

      if (to_server)
      {
        logger.addAppender<remote::RemoteAppender>
          (*to_server, remote_log_io_service);
      }

      logger.setLevel (from_string (level));
    }

    void configure_to_stderr (Logger& logger)
    {
      logger.addAppender<StreamAppender> (std::cerr, default_format::SHORT());
      logger.setLevel (TRACE);
    }
  }
}
