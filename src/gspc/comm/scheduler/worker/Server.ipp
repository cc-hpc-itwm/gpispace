#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/this_bound_mem_fn.hpp>

#include <utility>

namespace gspc
{
  namespace comm
  {
    namespace scheduler
    {
      namespace worker
      {
        template<typename Submit, typename Cancel>
            Server::Server (Submit&& submit, Cancel&& cancel)
          : _service_dispatcher()
          , _io_service (1)
          , _submit (_service_dispatcher, std::forward<Submit> (submit))
          , _cancel (_service_dispatcher, std::forward<Cancel> (cancel))
          , _service_socket_provider (_io_service, _service_dispatcher)
          , _service_tcp_provider (_io_service, _service_dispatcher)
          , _local_endpoint ( fhg::util::connectable_to_address_string
                                (_service_tcp_provider.local_endpoint())
                            , _service_socket_provider.local_endpoint()
                            )
        {}

        template<typename That>
          Server::Server (That* that)
            : Server ( fhg::util::bind_this (that, &That::submit)
                     , fhg::util::bind_this (that, &That::cancel)
                     )
        {}
      }
    }
  }
}
