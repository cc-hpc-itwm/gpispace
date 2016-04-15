#pragma once

#include <fhgcom/peer.hpp>

#include <sdpa/com/NetworkStrategy.hpp>
#include <sdpa/events/Codec.hpp>
#include <sdpa/events/SDPAEvent.hpp>

#include <functional>

namespace sdpa
{
  namespace com
  {
    class NetworkStrategy
    {
    public:
      NetworkStrategy ( std::function<void (fhg::com::p2p::address_t const&, sdpa::events::SDPAEvent::Ptr)> event_handler
                      , std::function<void (fhg::com::p2p::address_t const&, std::exception_ptr const&)> on_error
                      , std::unique_ptr<boost::asio::io_service> peer_io_service
                      , fhg::com::host_t const & host
                      , fhg::com::port_t const & port
                      )
        : _codec()
        , _event_handler (event_handler)
        , _on_error (std::move (on_error))
        , _peer (std::move (peer_io_service), host, port)
      {
        _peer.start_recv
          ( [this] ( fhg::com::p2p::address_t const& source
                   , std::string const& data
                   )
            {
              _event_handler
                (source, sdpa::events::SDPAEvent::Ptr (_codec.decode (data)));
            }
          , _on_error
          );
      }

      fhg::com::p2p::address_t connect_to
        (fhg::com::host_t const& host, fhg::com::port_t const& port)
      {
        return _peer.connect_to (host, port);
      }

      template<typename Event, typename... Args>
        void perform (fhg::com::p2p::address_t const& address, Args... args)
      {
        _peer.async_send ( address
                         , _codec.encode<Event> (std::forward<Args> (args)...)
                         , _on_error
                         );
      }

      boost::asio::ip::tcp::endpoint local_endpoint() const
      {
        return _peer.local_endpoint();
      }

    private:
      sdpa::events::Codec _codec;

      std::function<void (fhg::com::p2p::address_t const&, sdpa::events::SDPAEvent::Ptr)> _event_handler;
      std::function<void (fhg::com::p2p::address_t const&, std::exception_ptr const&)> _on_error;

      fhg::com::peer_t _peer;
    };
  }
}
