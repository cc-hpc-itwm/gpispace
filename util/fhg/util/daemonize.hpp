// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_DAEMONIZE_HPP
#define FHG_UTIL_DAEMONIZE_HPP

#include <boost/optional.hpp>

namespace fhg
{
  namespace util
  {
    void fork_and_daemonize_child_and_abandon_parent();
    //! \note returns boost::none in child
    boost::optional<pid_t> fork_and_daemonize_child();
  }
}

#endif
