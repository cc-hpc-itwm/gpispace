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

#include <drts/certificates.hpp>

#include <fhgcom/address.hpp>
#include <fhgcom/connection.hpp>
#include <fhgcom/message.hpp>
#include <fhgcom/peer_info.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/optional.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <algorithm>
#include <deque>
#include <exception>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>

namespace boost
{
  namespace asio
  {
    namespace ssl
    {
      class context;
    }
  }
}

namespace fhg
{
  namespace com
  {
    using Certificates = gspc::Certificates;

    /*!
      This class abstracts from an endpoint
     */
    class peer_t
    {
    public:
      using handler_t =
        std::function<void (const ::boost::system::error_code&)>;

      peer_t ( std::unique_ptr<::boost::asio::io_service>
             , host_t const& host
             , port_t const& port
             , Certificates const& certificates
             );

      virtual ~peer_t ();
      peer_t (peer_t const&) = delete;
      peer_t (peer_t&&) = delete;
      peer_t& operator= (peer_t const&) = delete;
      peer_t& operator= (peer_t&&) = delete;

      p2p::address_t const& address () const { return my_addr_.get(); }
      ::boost::asio::ip::tcp::endpoint local_endpoint() const
      {
        return acceptor_.local_endpoint();
      }

      p2p::address_t connect_to (host_t const&, port_t const&);

      void async_send ( p2p::address_t const& addr
                      , std::string const& data
                      , handler_t h
                      );
      void send ( p2p::address_t const& addr
                , std::string const& data
                );

      struct Received
      {
        Received (::boost::system::error_code);
        Received ( ::boost::system::error_code
                 , p2p::address_t
                 , message_t
                 );

        ::boost::system::error_code ec() const;
        p2p::address_t const& source() const;
        message_t const& message() const;

      private:
        ::boost::system::error_code _ec;
        ::boost::optional<p2p::address_t> _source;
        ::boost::optional<message_t> _message;
      };

      void async_recv (std::function<void (Received)>);

      std::exception_ptr TESTING_ONLY_handshake_exception() const;

    protected:
      void handle_hello_message (connection_t::ptr_t, std::unique_ptr<message_t> m);
      void handle_user_data   (connection_t::ptr_t, std::unique_ptr<message_t> m);
      void handle_error       (connection_t::ptr_t, ::boost::system::error_code const& error);

    private:
      struct to_send_t
      {
        // \note Only valid until an async request was started,
        // i.e. before passing to connection::async_send().
        // Afterwards, only handler remains valid.
        // \todo Also move handler into connection::async_send to not
        // require split state.
        message_t  message;
        handler_t  handler;
      };

      struct connection_data_t
      {
        bool send_in_progress {false};
        connection_t::ptr_t connection;
        std::deque<to_send_t> o_queue;
      };

      using mutex_type = std::recursive_mutex;
      using lock_type = std::unique_lock<mutex_type>;

      void accept_new ();
      void handle_accept (::boost::system::error_code const&);
      void connection_established (lock_type const&, connection_data_t&);
      void handle_send (p2p::address_t, ::boost::system::error_code const&);
      void start_sender (lock_type const&, connection_data_t&);

      friend class connection_t;

      void request_handshake_response
        ( p2p::address_t addr
        , std::shared_ptr<std::promise<std::exception_ptr>> connect_done
        , ::boost::system::error_code const& ec
        );
      void acknowledge_handshake_response
        (connection_t::ptr_t connection, ::boost::system::error_code const& ec);

      mutex_type mutex_;

      bool stopping_ {false};
      ::boost::optional<p2p::address_t> my_addr_;

      std::unique_ptr<::boost::asio::ssl::context> ctx_;

      std::unique_ptr<::boost::asio::io_service> io_service_;
      ::boost::asio::io_service::strand strand_;
      ::boost::asio::io_service::work io_service_work_;
      ::boost::asio::ip::tcp::acceptor acceptor_;

      std::unordered_map<p2p::address_t, connection_data_t> connections_;

      std::set<connection_t::ptr_t> backlog_;

      connection_t::ptr_t listen_;

      std::list<std::function<void (Received)>> m_to_recv;
      std::list<message_t> m_pending;

      std::exception_ptr TESTING_ONLY_handshake_exception_;
      ::boost::strict_scoped_thread<> _io_thread;
    };
  }
}
