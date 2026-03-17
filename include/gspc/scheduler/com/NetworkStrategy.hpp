// Copyright (C) 2010,2013-2016,2018-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/events/Codec.hpp>
#include <gspc/scheduler/events/SchedulerEvent.hpp>

#include <gspc/com/peer.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <optional>
#include <boost/system/error_code.hpp>

#include <functional>
#include <memory>
#include <string>


  namespace gspc::scheduler::com
  {
    class NetworkStrategy
    {
    public:
      using EventHandler
        = std::function < void ( gspc::com::p2p::address_t const&
                               , gspc::scheduler::events::SchedulerEvent::Ptr
                               )
                        >;

      NetworkStrategy ( EventHandler event_handler
                      , std::unique_ptr<::boost::asio::io_service> peer_io_service
                      , gspc::com::host_t const& host
                      , gspc::com::port_t const& port
                      , gspc::Certificates const& certificates
                      );
      ~NetworkStrategy();
      NetworkStrategy (NetworkStrategy const&) = delete;
      NetworkStrategy (NetworkStrategy&&) = delete;
      NetworkStrategy& operator= (NetworkStrategy const&) = delete;
      NetworkStrategy& operator= (NetworkStrategy&&) = delete;

      template<typename Event, typename... Args>
        void perform (gspc::com::p2p::address_t const& address, Args&&... args);

      ::boost::asio::ip::tcp::endpoint local_endpoint() const;

    private:
      gspc::scheduler::events::Codec _codec;
      void handle_recv (gspc::com::peer_t::Received);

      void perform ( gspc::com::p2p::address_t const& address
                   , std::string const& serialized_event
                   );

      EventHandler _event_handler;

      bool m_shutting_down {false};

      // there is a TESTING_ONLY class that adds behavior
    protected:
      gspc::com::peer_t _peer;
    };
  }


#include <gspc/scheduler/com/NetworkStrategy.ipp>
