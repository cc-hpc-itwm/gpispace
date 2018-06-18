#pragma once

#include <logging/legacy/receiver.hpp>
#include <logging/message.hpp>
#include <logging/tcp_endpoint.hpp>

#include <rpc/function_description.hpp>
#include <rpc/remote_tcp_endpoint.hpp>
#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>
#include <rpc/service_tcp_provider.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <list>

namespace fhg
{
  namespace logging
  {
    class legacy_bridge final : public legacy::receiver
    {
      struct protocol
      {
        FHG_RPC_FUNCTION_DESCRIPTION (sink, void (message));
        FHG_RPC_FUNCTION_DESCRIPTION (register_sink, void (tcp_endpoint));
      };

    public:
      legacy_bridge (unsigned short legacy_port);

      tcp_endpoint local_endpoint() const;

      class receiver
      {
      public:
        using callback_t = std::function<void (message const&)>;
        receiver (tcp_endpoint const& bridge, callback_t);

      private:
        callback_t _callback;

        rpc::service_dispatcher _service_dispatcher;
        util::scoped_boost_asio_io_service_with_threads _io_service;
        rpc::service_handler<protocol::sink> const _sink;
        fhg::rpc::service_tcp_provider const _service_provider;
      };

    private:
      virtual void on_legacy (legacy::event const&) override;

      rpc::service_dispatcher _service_dispatcher;
      util::scoped_boost_asio_io_service_with_threads _io_service;

      std::list<rpc::remote_tcp_endpoint> _sinks;
      rpc::service_handler<protocol::register_sink> const _register_sink;

      rpc::service_tcp_provider const _service_provider;
    };
  }
}
