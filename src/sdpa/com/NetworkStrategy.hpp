// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
