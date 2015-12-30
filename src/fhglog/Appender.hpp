// alexander.petry@itwm.fraunhofer.de

#pragma once

#include <fhglog/event.hpp>

namespace fhg
{
  namespace log
  {
    class Appender
    {
    public:
      virtual ~Appender() {}

      virtual void append (const LogEvent&) = 0;
      virtual void flush() = 0;
    };
  }
}
