// alexander.petry@itwm.fraunhofer.de

#ifndef FHG_LOG_APPENDER_HPP
#define FHG_LOG_APPENDER_HPP 1

#include <fhglog/event.hpp>

#include <boost/shared_ptr.hpp>

namespace fhg
{
  namespace log
  {
    class Appender
    {
    public:
      typedef boost::shared_ptr<Appender> ptr_t;

      virtual ~Appender() {}

      virtual void append (const LogEvent&) = 0;
      virtual void flush() = 0;
    };
  }
}

#endif
