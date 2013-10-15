#ifndef FHGLOG_FILTER_HPP
#define FHGLOG_FILTER_HPP 1

#include <fhglog/memory.hpp>
#include <fhglog/LogEvent.hpp>

namespace fhg { namespace log {
  class Filter {
  public:
    typedef shared_ptr<Filter> ptr_t;

    virtual bool operator()(const LogEvent &event) const = 0;
    virtual ~Filter() {} // make the compiler be happy
  };

  class FilterChain : public Filter {
  public:
    void addFilter(const Filter::ptr_t &filter)
    {
      filter_list_.push_back(filter);
    }

    bool operator()(const LogEvent &event) const
    {
      for (filter_list_t::const_iterator filter(filter_list_.begin()); filter != filter_list_.end(); ++filter)
      {
        if ((**filter)(event))
          return true;
      }
      return false;
    }

  private:
    typedef std::list<Filter::ptr_t> filter_list_t;
    filter_list_t filter_list_;
  };

  class LevelFilter : public Filter {
  public:
    explicit
    LevelFilter(const LogLevel &level)
      : level_(level) {}

    bool operator()(const LogEvent &evt) const
    {
      return evt.severity() < level_;
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
