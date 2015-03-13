#pragma once

#include <list>
#include <string>

#include <fhglog/level.hpp>
#include <fhglog/event.hpp>
#include <fhglog/Appender.hpp>

#include <memory>

namespace fhg
{
  namespace log
  {
    class Logger
    {
    public:
      Logger();

      void setLevel (const Level& level);
      bool isLevelEnabled (const Level& level) const
      {
        return level >= lvl_;
      }

      void log (const LogEvent &event);
      void flush();

      template<class A, class... Args>
      void addAppender (Args&&... args)
      {
        appenders_.emplace_back (new A (std::forward<Args> (args)...));
      }

    private:
      Level lvl_;

      std::list<std::unique_ptr<Appender>> appenders_;
    };

    Logger& GLOBAL_logger();
  }
}
