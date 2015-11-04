#pragma once

#include <fhgcom/connection.hpp>
#include <fhgcom/header.hpp>
#include <fhgcom/peer_info.hpp>

#include <fhg/util/thread/event.hpp>

#include <boost/optional.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <deque>
#include <list>
#include <set>
#include <string>
#include <unordered_map>

namespace fhg
{
  namespace com
  {
    /*!
      This class abstracts from an endpoint
     */
    class peer_t : private boost::noncopyable
    {
    public:
      typedef std::function <void (boost::system::error_code const &)> handler_t;

      peer_t (std::unique_ptr<boost::asio::io_service>, host_t const& host, port_t const& port);

      virtual ~peer_t ();

      p2p::address_t const & address () const { return my_addr_.get(); }
      boost::asio::ip::tcp::endpoint local_endpoint() const
      {
        return acceptor_.local_endpoint();
      }

      p2p::address_t connect_to (host_t const&, port_t const&);
      p2p::address_t connect_to_or_use_existing_connection
        (host_t const&, port_t const&);

      void async_send ( p2p::address_t const& addr
                      , std::string const & data
                      , handler_t h
                      );
      void send ( p2p::address_t const& addr
                , std::string const & data
                );

      void async_recv
        ( message_t *m
        , std::function<void ( boost::system::error_code
                             , boost::optional<fhg::com::p2p::address_t> source
                             )
                       >
        );
      void recv (message_t *m);

    protected:
      void handle_hello_message (connection_t::ptr_t, const message_t *m);
      void handle_user_data   (connection_t::ptr_t, const message_t *m);
      void handle_error       (connection_t::ptr_t, const boost::system::error_code & error);

    private:
      struct to_send_t
      {
        to_send_t ()
          : message ()
          , handler()
        {}

        message_t  message;
        handler_t  handler;
      };

      struct to_recv_t
      {
        to_recv_t ()
          : message (nullptr)
          , handler()
        {}

        message_t  *message;
        std::function<void ( boost::system::error_code
                           , boost::optional<fhg::com::p2p::address_t>
                           )
                     > handler;
      };

      struct connection_data_t
      {
        connection_data_t ()
          : send_in_progress (false)
        {}

        bool send_in_progress;
        connection_t::ptr_t connection;
        std::deque<to_send_t> o_queue;
      };

      void accept_new ();
      void handle_accept (const boost::system::error_code &);
      void connection_established (const p2p::address_t, boost::system::error_code const &);
      void handle_send (const p2p::address_t, const boost::system::error_code &);
      void start_sender (const p2p::address_t);

      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      mutable mutex_type mutex_;

      bool stopping_;
      std::string host_;
      std::string port_;
      boost::optional<p2p::address_t> my_addr_;

      std::unique_ptr<boost::asio::io_service> io_service_;
      boost::asio::io_service::work io_service_work_;
      boost::asio::ip::tcp::acceptor acceptor_;

      std::unordered_map<p2p::address_t, connection_data_t> connections_;

      std::set<connection_t::ptr_t> backlog_;

      connection_t::ptr_t listen_;

      std::list<to_recv_t> m_to_recv;
      std::list<const message_t *> m_pending;

      boost::strict_scoped_thread<boost::join_if_joinable> _io_thread;
    };
  }
}
