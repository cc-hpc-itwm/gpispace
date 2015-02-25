#pragma once

#include <list>
#include <string>

#include <fhglog/level.hpp>
#include <fhglog/event.hpp>
#include <fhglog/Appender.hpp>

#include <boost/shared_ptr.hpp>

/**
   Common logging framework.

   Logger logger(Logger::get());

   logger.log(LogEvent(...));
*/
namespace fhg
{
  namespace log
  {
    class Logger
    {
    public:
      typedef boost::shared_ptr<Logger> ptr_t;

      static Logger::ptr_t get();

      explicit Logger (const std::string &name);
      Logger (const std::string& name, const Logger& inherit_from);

      void setLevel (const Level& level);
      bool isLevelEnabled (const Level& level) const
      {
        return level >= lvl_;
      }

      void log (const LogEvent &event);
      void flush();

      void addAppender(Appender::ptr_t);

    private:
      std::string name_;
      Level lvl_;

      std::list<Appender::ptr_t> appenders_;
    };
  }
}
