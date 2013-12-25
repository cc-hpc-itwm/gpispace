#ifndef FHGLOG_FILTER_HPP
#define FHGLOG_FILTER_HPP 1

#include <fhglog/event.hpp>

#include <boost/shared_ptr.hpp>

#include <list>

namespace fhg { namespace log {
  class Filter {
  public:
    typedef boost::shared_ptr<Filter> ptr_t;

    virtual bool operator()(const LogEvent &event) const = 0;
    virtual ~Filter() {}
  };

  class LevelFilter : public Filter {
  public:
    explicit
    LevelFilter(const LogLevel &level)
      : level_(level) {}

    bool operator()(const LogEvent &evt) const
    {
      return evt.severity().lvl() < level_.lvl();
    }
  private:
    LogLevel level_;
  };

  class NullFilter : public Filter {
    bool operator()(const LogEvent &) const
    {
      return false;
    }
  };
}}

#endif
