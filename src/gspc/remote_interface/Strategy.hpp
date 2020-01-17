#pragma once

#include <gspc/remote_interface/strategy/Thread.hpp>

#include <boost/variant.hpp>

namespace gspc
{
  namespace remote_interface
  {
    //! Strategy must not contain state but only information
    //! (e.g. pointer, filename, ...) about to get a state, Strategies
    //! _are_ copied!
    using Strategy = boost::variant< strategy::Thread
                                   // , strategy::ssh
                                   >;
  }
}
