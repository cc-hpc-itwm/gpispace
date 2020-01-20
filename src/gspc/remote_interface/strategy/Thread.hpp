#pragma once

#include <gspc/RemoteInterface.hpp>
#include <gspc/remote_interface/Hostname.hpp>
#include <gspc/remote_interface/ID.hpp>
#include <gspc/rpc/TODO.hpp>

#include <memory>
#include <unordered_map>

namespace gspc
{
  namespace remote_interface
  {
    namespace strategy
    {
      class Thread
      {
        struct RemoteInterfaceServer
        {
          RemoteInterfaceServer (ID);

          rpc::endpoint local_endpoint() const;

        private:
          RemoteInterface _remote_interface;
        };

      public:
        using State = std::unordered_map<Hostname, RemoteInterfaceServer>;

        Thread (std::shared_ptr<State>);
        rpc::endpoint boot (Hostname, ID) const;
        void teardown (Hostname) const;

        friend bool operator== (Thread const&, Thread const&);

      private:
        std::shared_ptr<State> _remote_interfaces_by_hostname;
      };
    }
  }
}
