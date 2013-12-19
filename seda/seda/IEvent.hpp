#ifndef SEDA_IEVENT_HPP
#define SEDA_IEVENT_HPP

#include <boost/shared_ptr.hpp>

namespace seda
{
  class IEvent
  {
  public:
    virtual ~IEvent() {}
  };
}

#endif
