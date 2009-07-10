#ifndef FHG_LOG_APPENDER_HPP
#define FHG_LOG_APPENDER_HPP 1

#include <tr1/memory>

namespace fhg { namespace log {
  class Appender {
  public:
    typedef std::tr1::shared_ptr<Appender> ptr_t;
  };
}}

#endif
