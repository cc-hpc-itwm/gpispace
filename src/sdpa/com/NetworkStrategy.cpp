// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <sdpa/com/NetworkStrategy.hpp>

#include <sdpa/events/ErrorEvent.hpp>

#include <util-generic/print_exception.hpp>
#include <util-generic/this_bound_mem_fn.hpp>

#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <utility>

namespace sdpa
{
  namespace com
  {
    NetworkStrategy::NetworkStrategy
        ( EventHandler event_handler
        , std::unique_ptr<boost::asio::io_service> peer_io_service
        , fhg::com::host_t const& host
        , fhg::com::port_t const& port
        , fhg::com::Certificates const& certificates
        )
      : _codec()
      , _event_handler (event_handler)
      , m_shutting_down (false)
      , _peer (std::move (peer_io_service), host, port, certificates)
    {
      _peer.async_recv
        (fhg::util::bind_this (this, &NetworkStrategy::handle_recv));
    }

    NetworkStrategy::~NetworkStrategy()
    {
      m_shutting_down = true;
    }

    fhg::com::p2p::address_t NetworkStrategy::connect_to
      (fhg::com::host_t const& host, fhg::com::port_t const& port)
    {
      return _peer.connect_to (host, port);
    }

    boost::asio::ip::tcp::endpoint NetworkStrategy::local_endpoint() const
    {
      return _peer.local_endpoint();
    }

    void NetworkStrategy::handle_recv
      ( boost::system::error_code const& ec
      , boost::optional<fhg::com::p2p::address_t> source
      , fhg::com::message_t message
      )
    {
      if (!ec)
      {
        events::SDPAEvent::Ptr const evt
          ( _codec.decode
              (std::string (message.data.begin(), message.data.end()))
          );
        _event_handler (source.get(), evt);

        _peer.async_recv
          (fhg::util::bind_this (this, &NetworkStrategy::handle_recv));
      }
      else if (!m_shutting_down)
      {
        if (message.header.src != _peer.address())
        {
          events::ErrorEvent::Ptr const error
            ( boost::make_shared<events::ErrorEvent>
                ( events::ErrorEvent::SDPA_ENODE_SHUTDOWN
                , ec.message()
                )
            );
          _event_handler
            (source.get_value_or (fhg::com::p2p::address_t ("unknown")), error);

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
          , [address, this] (boost::system::error_code const& ec)
            {
              if (ec)
              {
                _event_handler
                  ( address
                  , boost::make_shared<events::ErrorEvent>
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
          , boost::make_shared<events::ErrorEvent>
              ( events::ErrorEvent::SDPA_ENODE_SHUTDOWN
              , fhg::util::current_exception_printer (": ").string()
              )
          );
      }
    }
  }
}
