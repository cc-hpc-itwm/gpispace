#ifndef FHG_LOG_APPENDER_HPP
#define FHG_LOG_APPENDER_HPP 1

#include    <fhglog/memory.hpp>
#include    <fhglog/Formatter.hpp>

namespace fhg { namespace log {
  class Appender {
    public:
      typedef shared_ptr<Appender> ptr_t;

      virtual ~Appender() {}

      virtual void append(const LogEvent &evt) const = 0;

      virtual inline void setFormat(const Formatter::ptr_t &fmt) { fmt_ = fmt; }
      // takes ownership!
      virtual inline void setFormat(Formatter *fmt) { fmt_ = Formatter::ptr_t(fmt); }
      virtual inline const Formatter::ptr_t &getFormat() const { return fmt_; }

      const std::string &name() const { return name_; }

      const Appender &operator<<(const LogEvent &evt) const { this->append(evt); return *this; }
    protected:
      explicit
      Appender(const std::string &a_name)
        : name_(a_name), fmt_(Formatter::Default())
      {
      }
    private:
      std::string name_;
      Formatter::ptr_t fmt_;
  };
}}

#endif
