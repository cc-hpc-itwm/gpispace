#ifndef FHG_LOG_APPENDER_HPP
#define FHG_LOG_APPENDER_HPP 1

#include    <string>

#include    <fhglog/memory.hpp>
#include    <fhglog/LogEvent.hpp>

namespace fhg { namespace log {
  class Appender {
    public:
      typedef shared_ptr<Appender> ptr_t;

      virtual ~Appender() {}

      virtual void append(const LogEvent &evt) = 0;
      virtual void flush (void) = 0;

      const std::string &name() const { return name_; }
    protected:
      explicit
      Appender(const std::string &a_name)
        : name_(a_name)
      { }
    private:
      std::string name_;
  };
}}

#endif
