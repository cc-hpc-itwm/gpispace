#ifndef FHG_LOG_LOGGER_HPP
#define FHG_LOG_LOGGER_HPP 1

#include <list>
#include <string>

#include <fhglog/level.hpp>
#include <fhglog/event.hpp>
#include <fhglog/Appender.hpp>

#include <boost/shared_ptr.hpp>

/**
   Common logging framework.

   Logger root_logger(Logger::get());
   Logger my_logger(Logger::get("module"));

   my_logger.log(LogEvent(...));
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
      static Logger::ptr_t get (const std::string&name);

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

#endif
