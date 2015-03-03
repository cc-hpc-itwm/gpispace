#pragma once

#include <sdpa/events/SDPAEvent.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <fhgcom/peer.hpp>

#include <functional>

namespace sdpa
{
  namespace com
  {
    class NetworkStrategy
    {
    public:
      NetworkStrategy ( std::function<void (fhg::com::p2p::address_t const&, sdpa::events::SDPAEvent::Ptr)> event_handler
                      , std::unique_ptr<boost::asio::io_service> peer_io_service
                      , fhg::com::host_t const & host
                      , fhg::com::port_t const & port
                      , fhg::log::Logger& peer_logger
                      );
      ~NetworkStrategy();

      fhg::com::p2p::address_t connect_to
        (fhg::com::host_t const&, fhg::com::port_t const&);

      void perform ( fhg::com::p2p::address_t const&
                   , boost::shared_ptr<events::SDPAEvent> const&
                   );

      boost::asio::ip::tcp::endpoint local_endpoint() const
      {
        return _peer.local_endpoint();
      }

    private:
      void handle_recv ( boost::system::error_code const & ec
                       , boost::optional<fhg::com::p2p::address_t> source_name
                       );

      std::function<void (fhg::com::p2p::address_t const&, sdpa::events::SDPAEvent::Ptr)> _event_handler;

      fhg::com::message_t m_message;
      bool m_shutting_down;

      fhg::com::peer_t _peer;
    };
  }
}
