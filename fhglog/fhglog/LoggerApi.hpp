// alexander.petry@itwm.fraunhofer.de

#ifndef FHGLOG_LOGGER_API_HPP
#define FHGLOG_LOGGER_API_HPP 1

#include <string>
#include <fhglog/level.hpp>
#include <fhglog/Appender.hpp>
#include <fhglog/Logger.hpp>

namespace fhg
{
  namespace log
  {
    class LoggerApi;

    LoggerApi getLogger();
    LoggerApi getLogger (const std::string &name);
    LoggerApi getLogger (const std::string &name, const std::string& base);

    class LoggerApi
    {
      friend LoggerApi getLogger();
      friend LoggerApi getLogger (const std::string&);
      friend LoggerApi getLogger (const std::string&, const std::string&);

    public:
      void setLevel (const LogLevel& level)
      {
        impl_->setLevel (level);
      }
      bool isLevelEnabled (const LogLevel& level) const
      {
        return impl_->isLevelEnabled (level);
      }

      void setFilter (const Filter::ptr_t& filter)
      {
        impl_->setFilter(filter);
      }
      bool isFiltered (const LogEvent& event) const
      {
        return impl_->isFiltered(event);
      }
      void addAppender (Appender::ptr_t appender)
      {
        impl_->addAppender(appender);
      }

      void log (const LogEvent& event)
      {
        impl_->log(event);
      }
      void flush()
      {
        impl_->flush();
      }

    private:
      explicit LoggerApi (Logger::ptr_t impl)
        : impl_ (impl)
      {}

      Logger::ptr_t impl_;
    };

    inline LoggerApi getLogger()
    {
      return LoggerApi (Logger::get());
    }
    inline LoggerApi getLogger (const std::string &name)
    {
      return LoggerApi (Logger::get (name));
    }
    inline LoggerApi getLogger( const std::string &name
                              , const std::string &base
                              )
    {
      return LoggerApi (Logger::get (name, base));
    }
  }
}

#endif
