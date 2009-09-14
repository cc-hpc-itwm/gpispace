#ifndef FHG_LOG_APPENDER_HPP
#define FHG_LOG_APPENDER_HPP 1

#include    <tr1/memory>
#include    <fhglog/Formatter.hpp>

namespace fhg { namespace log {
  class Appender {
    public:
      typedef std::tr1::shared_ptr<Appender> ptr_t;

      virtual ~Appender() {}

      virtual void append(const LogEvent &evt) const = 0;

      inline void setFormat(Formatter::ptr_t fmt) { fmt_ = fmt; }
      // takes ownership!
      inline void setFormat(Formatter *fmt) { fmt_ = Formatter::ptr_t(fmt); }
      inline const Formatter::ptr_t getFormat() const { return fmt_; }

      const std::string &name() const { return name_; }
    protected:
      explicit
      Appender(const std::string &name)
        : name_(name), fmt_(Formatter::Default())
      {
      }
    private:
      std::string name_;
      Formatter::ptr_t fmt_;
  };
}}

#endif
