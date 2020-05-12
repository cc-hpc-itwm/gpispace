#include <logging/demultiplexer.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/this_bound_mem_fn.hpp>

namespace fhg
{
  namespace logging
  {
    demultiplexer::demultiplexer
        (rif::started_process_promise& promise, int, char**)
      : receiver ( [&] (boost::asio::yield_context yield, message const& m)
                   {
                     return emitter.emit_message (m, std::move (yield));
                   }
                 )
      , add_emitters
          ( service_dispatcher
          , util::bind_this (&receiver, &stream_receiver::add_emitters)
          , fhg::rpc::yielding
          )
      , add_service_provider (io_service, service_dispatcher)
    {
      promise.set_result
        ( emitter.local_endpoint().to_string()
        , socket_endpoint (add_service_provider.local_endpoint()).to_string()
        );
    }
  }
}
