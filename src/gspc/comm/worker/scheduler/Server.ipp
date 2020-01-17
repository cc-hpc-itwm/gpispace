#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/this_bound_mem_fn.hpp>

#include <utility>

namespace gspc
{
  namespace comm
  {
    namespace worker
    {
      namespace scheduler
      {
        template<typename Finished>
            Server::Server (Finished&& finished)
          : _service_dispatcher()
          , _io_service (1)
          , _finished (_service_dispatcher, std::forward<Finished> (finished))
          , _service_socket_provider (_io_service, _service_dispatcher)
          , _service_tcp_provider (_io_service, _service_dispatcher)
          , _local_endpoint ( fhg::util::connectable_to_address_string
                                (_service_tcp_provider.local_endpoint())
                            , _service_socket_provider.local_endpoint()
                            )
        {}

        template<typename That>
          Server::Server (That* that)
            : Server (fhg::util::bind_this (that, &That::finished))
        {}
      }
    }
  }
}
