#include <fhglog/Logger.hpp>

namespace fhg
{
  namespace log
  {
    Logger::ptr_t GLOBAL_logger()
    {
      static Logger::ptr_t l (new Logger());

      return l;
    }

    Logger::Logger()
      : lvl_ (INFO)
    {}

    void Logger::setLevel (const Level& level)
    {
      lvl_ = level;
    }

    void Logger::log (const LogEvent& event)
    {
      if (isLevelEnabled (event.severity()))
      {
        for (std::unique_ptr<Appender> const& appender : appenders_)
        {
          appender->append(event);
        }
      }
    }

    void Logger::flush()
    {
      for (std::unique_ptr<Appender> const& appender : appenders_)
      {
        appender->flush();
      }
    }
  }
}
