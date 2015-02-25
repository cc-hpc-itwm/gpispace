#include <fhglog/Logger.hpp>

namespace fhg
{
  namespace log
  {
    Logger::ptr_t Logger::get()
    {
      static Logger::ptr_t l (new Logger ("default"));

      return l;
    }

    Logger::Logger (const std::string& name)
      : name_ (name)
      , lvl_ (INFO)
    {}

    void Logger::setLevel (const Level& level)
    {
      lvl_ = level;
    }

    void Logger::log (const LogEvent& event)
    {
      if (isLevelEnabled (event.severity()))
      {
        for (Appender::ptr_t const& appender : appenders_)
        {
          appender->append(event);
        }
      }
    }

    void Logger::flush()
    {
      for (Appender::ptr_t const& appender : appenders_)
      {
        appender->flush();
      }
    }

    void Logger::addAppender (Appender::ptr_t appender)
    {
      appenders_.push_back(appender);
    }
  }
}
