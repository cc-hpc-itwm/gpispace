#pragma once

#include <sdpa/events/Codec.hpp>
#include <sdpa/events/SDPAEvent.hpp>

#include <fhgcom/peer.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/optional.hpp>
#include <boost/system/error_code.hpp>

#include <functional>
#include <memory>
#include <string>

namespace sdpa
{
  namespace com
  {
    class NetworkStrategy
    {
    public:
      using EventHandler
        = std::function < void ( fhg::com::p2p::address_t const&
                               , sdpa::events::SDPAEvent::Ptr
                               )
                        >;

      NetworkStrategy ( EventHandler event_handler
                      , std::unique_ptr<boost::asio::io_service> peer_io_service
                      , fhg::com::host_t const& host
                      , fhg::com::port_t const& port
                      , fhg::com::Certificates const& certificates
                      );
      ~NetworkStrategy();

      fhg::com::p2p::address_t connect_to
        (fhg::com::host_t const&, fhg::com::port_t const&);

      template<typename Event, typename... Args>
        void perform (fhg::com::p2p::address_t const& address, Args&&... args);

      boost::asio::ip::tcp::endpoint local_endpoint() const;

    private:
      sdpa::events::Codec _codec;
      void handle_recv ( boost::system::error_code const& ec
                       , boost::optional<fhg::com::p2p::address_t> source_name
                       );

      void perform ( fhg::com::p2p::address_t const& address
                   , std::string const& serialized_event
                   );

      EventHandler _event_handler;

      fhg::com::message_t m_message;
      bool m_shutting_down;

      fhg::com::peer_t _peer;
    };
  }
}

#include <sdpa/com/NetworkStrategy.ipp>
