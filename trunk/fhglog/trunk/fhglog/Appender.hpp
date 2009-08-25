#ifndef FHG_LOG_APPENDER_HPP
#define FHG_LOG_APPENDER_HPP 1

#include    <tr1/memory>
#include    <fhglog/Formatter.hpp>

namespace fhg { namespace log {
  class Appender {
    public:
      typedef std::tr1::shared_ptr<Appender> ptr_t;

      virtual ~Appender() {}
      virtual void append(const LogEvent &evt) = 0;
      virtual void setFormat(Formatter::ptr_t fmt) = 0;
      virtual const std::string &name() const = 0;
  };
}}

#endif
