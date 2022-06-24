// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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
                      , std::unique_ptr<::boost::asio::io_service> peer_io_service
                      , fhg::com::host_t const& host
                      , fhg::com::port_t const& port
                      , fhg::com::Certificates const& certificates
                      );
      ~NetworkStrategy();
      NetworkStrategy (NetworkStrategy const&) = delete;
      NetworkStrategy (NetworkStrategy&&) = delete;
      NetworkStrategy& operator= (NetworkStrategy const&) = delete;
      NetworkStrategy& operator= (NetworkStrategy&&) = delete;

      template<typename Event, typename... Args>
        void perform (fhg::com::p2p::address_t const& address, Args&&... args);

      ::boost::asio::ip::tcp::endpoint local_endpoint() const;

    private:
      sdpa::events::Codec _codec;
      void handle_recv (fhg::com::peer_t::Received);

      void perform ( fhg::com::p2p::address_t const& address
                   , std::string const& serialized_event
                   );

      EventHandler _event_handler;

      bool m_shutting_down {false};

      // there is a TESTING_ONLY class that adds behavior
    protected:
      fhg::com::peer_t _peer;
    };
  }
}

#include <sdpa/com/NetworkStrategy.ipp>
