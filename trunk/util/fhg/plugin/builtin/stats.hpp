#ifndef FHG_PLUGIN_STATS_HPP
#define FHG_PLUGIN_STATS_HPP 1

#include <ostream>

namespace stats
{
  class Statistics
  {
  public:
    virtual ~Statistics () {}

    virtual void inc(const char * counter) = 0;
    virtual void dec(const char * counter) = 0;

    virtual void start_timer (const char * timer) = 0;
    virtual void stop_timer (const char * timer) = 0;

    virtual size_t count(const char *name) const = 0;
    virtual double timer(const char *name) const = 0;

    virtual void print(std::ostream &os) const = 0;
  };
}

#endif
