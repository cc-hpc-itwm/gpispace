#ifndef SDPA_PLUGIN_OBSERVER_HPP
#define SDPA_PLUGIN_OBSERVER_HPP 1

#include <boost/any.hpp>

namespace observe
{
  class Observer
  {
  public:
    virtual ~Observer() {}

    virtual void notify(boost::any const &) = 0;
  };
}

#endif
