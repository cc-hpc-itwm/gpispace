#include <gspc/remote_interface/strategy/Thread.hpp>

#include <stdexcept>
#include <tuple>

namespace gspc
{
  namespace remote_interface
  {
    namespace strategy
    {
      Thread::RemoteInterfaceServer::RemoteInterfaceServer (ID id)
        : _remote_interface (id)
      {}
      rpc::endpoint Thread::RemoteInterfaceServer::local_endpoint() const
      {
        return _remote_interface.local_endpoint();
      }

      Thread::Thread (std::shared_ptr<State> state)
        : _remote_interfaces_by_hostname {state}
      {}

      bool operator== (Thread const& lhs, Thread const& rhs)
      {
        return std::tie (lhs._remote_interfaces_by_hostname)
          == std::tie (rhs._remote_interfaces_by_hostname)
          ;
      }
      rpc::endpoint Thread::boot
        ( Hostname hostname
        , ID id
        ) const
      {
        auto const remote_interface
          (_remote_interfaces_by_hostname->emplace (hostname, id));

        if (!remote_interface.second)
        {
          throw std::invalid_argument
            (str ( boost::format ("strategy::Thread: Duplicate host '%1%'.")
                 % hostname
                 )
            );
        }

        return remote_interface.first->second.local_endpoint();
      }

      void Thread::teardown (Hostname hostname) const
      {
        if (!_remote_interfaces_by_hostname->erase (hostname))
        {
          throw std::invalid_argument
            (str ( boost::format ("strategy::Thread: Unknown host '%1%'.")
                 % hostname
                 )
            );
        }
      }
    }
  }
}
