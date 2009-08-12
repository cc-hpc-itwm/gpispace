#ifndef FHG_LOG_APPENDER_HPP
#define FHG_LOG_APPENDER_HPP 1

#include	<iostream>
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

  class ConsoleAppender : public Appender
  {
    public:
      explicit
      ConsoleAppender(std::ostream &stream = std::cerr)
        : name_("console-appender"), fmt_(new Formatter("default")), stream_(stream) {}
      ConsoleAppender(Formatter::ptr_t fmt, std::ostream &stream = std::cerr)
        : name_("console-appender"), fmt_(fmt), stream_(stream) {}
      ~ConsoleAppender() {}

      void setFormat(Formatter::ptr_t fmt) { fmt_ = fmt; }
      void append(const LogEvent &evt);
      const std::string &name() const { return name_; }
    private:
      std::string name_;
      Formatter::ptr_t fmt_;
      std::ostream &stream_;
  };
}}

#endif
