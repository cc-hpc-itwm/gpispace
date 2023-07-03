// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sdpa/com/NetworkStrategy.hpp>

#include <sdpa/events/ErrorEvent.hpp>

#include <util-generic/print_exception.hpp>
#include <util-generic/this_bound_mem_fn.hpp>

#include <algorithm>
#include <utility>

namespace sdpa
{
  namespace com
  {
    NetworkStrategy::NetworkStrategy
        ( EventHandler event_handler
        , std::unique_ptr<::boost::asio::io_service> peer_io_service
        , fhg::com::host_t const& host
        , fhg::com::port_t const& port
        , fhg::com::Certificates const& certificates
        )
      : _codec()
      , _event_handler (event_handler)
      , _peer (std::move (peer_io_service), host, port, certificates)
    {
      _peer.async_recv
        (fhg::util::bind_this (this, &NetworkStrategy::handle_recv));
    }

    NetworkStrategy::~NetworkStrategy()
    {
      m_shutting_down = true;
    }

    ::boost::asio::ip::tcp::endpoint NetworkStrategy::local_endpoint() const
    {
      return _peer.local_endpoint();
    }

    void NetworkStrategy::handle_recv (fhg::com::peer_t::Received received)
    {
      if (!received.ec())
      {
        auto const& source {received.source()};
        auto const& message {received.message()};
        events::SDPAEvent::Ptr const evt
          ( _codec.decode
              (std::string (message.data.begin(), message.data.end()))
          );
        _event_handler (source, evt);

        _peer.async_recv
          (fhg::util::bind_this (this, &NetworkStrategy::handle_recv));
      }
      else if (!m_shutting_down)
      {
        auto const& source {received.source()};
        auto const& message {received.message()};
        if (message.header.src != _peer.address())
        {
          events::ErrorEvent::Ptr const error
            ( ::boost::make_shared<events::ErrorEvent>
                ( events::ErrorEvent::SDPA_ENODE_SHUTDOWN
                , received.ec().message()
                )
            );
          _event_handler (source, error);

          _peer.async_recv
            (fhg::util::bind_this (this, &NetworkStrategy::handle_recv));
        }
      }
    }

    void NetworkStrategy::perform
      (fhg::com::p2p::address_t const& address, std::string const& event)
    {
      try
      {
        _peer.async_send
          ( address
          , event
          , [address, this] (::boost::system::error_code const& ec)
            {
              if (ec)
              {
                _event_handler
                  ( address
                  , ::boost::make_shared<events::ErrorEvent>
                      (events::ErrorEvent::SDPA_ENODE_SHUTDOWN, ec.message())
                  );
              }
            }
          );
      }
      catch (...)
      {
        _event_handler
          ( address
          , ::boost::make_shared<events::ErrorEvent>
              ( events::ErrorEvent::SDPA_ENODE_SHUTDOWN
              , fhg::util::current_exception_printer (": ").string()
              )
          );
      }
    }
  }
}
