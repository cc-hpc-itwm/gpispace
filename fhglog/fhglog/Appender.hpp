#ifndef FHG_LOG_APPENDER_HPP
#define FHG_LOG_APPENDER_HPP 1

#include    <fhglog/memory.hpp>
#include    <fhglog/event.hpp>

namespace fhg { namespace log {
  class Appender {
    public:
      typedef shared_ptr<Appender> ptr_t;

      virtual ~Appender() {}

      virtual void append(const LogEvent &evt) = 0;
      virtual void flush (void) = 0;
  };
}}

#endif
