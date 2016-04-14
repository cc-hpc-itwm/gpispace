#pragma once

#include <sdpa/events/Codec.hpp>
#include <sdpa/events/SDPAEvent.hpp>

#include <util-generic/print_exception.hpp>

#include <boost/make_shared.hpp>

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
                      , std::function<void (fhg::com::p2p::address_t const&, std::exception_ptr const&)> network_error_handler
                      , std::unique_ptr<boost::asio::io_service> peer_io_service
                      , fhg::com::host_t const & host
                      , fhg::com::port_t const & port
                      );
      ~NetworkStrategy();

      fhg::com::p2p::address_t connect_to
        (fhg::com::host_t const&, fhg::com::port_t const&);

      template<typename Event, typename... Args>
        void perform (fhg::com::p2p::address_t const& address, Args... args)
      {
        try
        {
          _peer.async_send
            ( address
            , _codec.encode<Event> (std::forward<Args> (args)...)
            , [address, this] (boost::system::error_code const& ec)
              {
                if (ec)
                {
                  _on_error (address, std::make_exception_ptr (boost::system::system_error (ec)));
                }
              }
            );
        }
        catch (...)
        {
          _on_error (address, std::current_exception());
        }
      }

      boost::asio::ip::tcp::endpoint local_endpoint() const
      {
        return _peer.local_endpoint();
      }

    private:
      sdpa::events::Codec _codec;
      void handle_recv ( boost::system::error_code const & ec
                       , boost::optional<fhg::com::p2p::address_t> source_name
                       );

      std::function<void (fhg::com::p2p::address_t const&, sdpa::events::SDPAEvent::Ptr)> _event_handler;
      std::function<void (fhg::com::p2p::address_t const&, std::exception_ptr const&)> _on_error;

      fhg::com::message_t m_message;
      bool m_shutting_down;

      fhg::com::peer_t _peer;
    };
  }
}
