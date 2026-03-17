// Copyright (C) 2010,2013-2016,2018-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/scheduler/com/NetworkStrategy.hpp>

#include <gspc/scheduler/events/ErrorEvent.hpp>

#include <gspc/util/print_exception.hpp>
#include <gspc/util/this_bound_mem_fn.hpp>

#include <algorithm>
#include <memory>
#include <utility>


  namespace gspc::scheduler::com
  {
    NetworkStrategy::NetworkStrategy
        ( EventHandler event_handler
        , std::unique_ptr<::boost::asio::io_service> peer_io_service
        , gspc::com::host_t const& host
        , gspc::com::port_t const& port
        , gspc::Certificates const& certificates
        )
      : _codec()
      , _event_handler (event_handler)
      , _peer (std::move (peer_io_service), host, port, certificates)
    {
      _peer.async_recv
        (gspc::util::bind_this (this, &NetworkStrategy::handle_recv));
    }

    NetworkStrategy::~NetworkStrategy()
    {
      m_shutting_down = true;
    }

    ::boost::asio::ip::tcp::endpoint NetworkStrategy::local_endpoint() const
    {
      return _peer.local_endpoint();
    }

    void NetworkStrategy::handle_recv (gspc::com::peer_t::Received received)
    {
      if (!received.ec())
      {
        auto const& source {received.source()};
        auto const& message {received.message()};
        events::SchedulerEvent::Ptr const evt
          ( _codec.decode
              (std::string (message.data.begin(), message.data.end()))
          );
        _event_handler (source, evt);

        _peer.async_recv
          (gspc::util::bind_this (this, &NetworkStrategy::handle_recv));
      }
      else if (!m_shutting_down)
      {
        auto const& source {received.source()};
        auto const& message {received.message()};
        if (message.header.src != _peer.address())
        {
          events::ErrorEvent::Ptr const error
            ( std::make_shared<events::ErrorEvent>
                ( events::ErrorEvent::SCHEDULER_ENODE_SHUTDOWN
                , received.ec().message()
                )
            );
          _event_handler (source, error);

          _peer.async_recv
            (gspc::util::bind_this (this, &NetworkStrategy::handle_recv));
        }
      }
    }

    void NetworkStrategy::perform
      (gspc::com::p2p::address_t const& address, std::string const& event)
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
                  , std::make_shared<events::ErrorEvent>
                      (events::ErrorEvent::SCHEDULER_ENODE_SHUTDOWN, ec.message())
                  );
              }
            }
          );
      }
      catch (...)
      {
        _event_handler
          ( address
          , std::make_shared<events::ErrorEvent>
              ( events::ErrorEvent::SCHEDULER_ENODE_SHUTDOWN
              , gspc::util::current_exception_printer (": ").string()
              )
          );
      }
    }
  }
