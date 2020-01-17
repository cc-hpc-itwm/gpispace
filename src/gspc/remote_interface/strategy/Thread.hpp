#pragma once

#include <gspc/RemoteInterface.hpp>
#include <gspc/remote_interface/Hostname.hpp>
#include <gspc/remote_interface/ID.hpp>
#include <gspc/rpc/TODO.hpp>

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

        Thread (State*);
        rpc::endpoint boot (Hostname, ID) const;
        void teardown (Hostname) const;

      private:
        State* _remote_interfaces_by_hostname;
      };
    }
  }
}
