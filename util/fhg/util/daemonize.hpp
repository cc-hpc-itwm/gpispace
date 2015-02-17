// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <boost/optional.hpp>

#include <initializer_list>

namespace boost
{
  namespace asio
  {
    class io_service;
  }
}

namespace fhg
{
  namespace util
  {
    void fork_and_daemonize_child_and_abandon_parent();
    void fork_and_daemonize_child_and_abandon_parent
      (std::initializer_list<boost::asio::io_service*>);
    //! \note returns boost::none in child
    boost::optional<pid_t> fork_and_daemonize_child();
  }
}
