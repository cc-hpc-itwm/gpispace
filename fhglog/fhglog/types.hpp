#ifndef FHG_LOG_TYPES_HPP
#define FHG_LOG_TYPES_HPP 1

#include <boost/function.hpp>
#include <fhglog/LogEvent.hpp>

namespace fhg
{
  namespace log
  {
    typedef boost::function<void (LogEvent const &)> sink_t;
    typedef boost::function<bool (LogEvent const &)> filter_t;
  }
}

#endif
