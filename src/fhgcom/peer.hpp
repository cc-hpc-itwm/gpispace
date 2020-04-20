#pragma once

#include <drts/certificates.hpp>

#include <fhgcom/connection.hpp>
#include <fhgcom/header.hpp>
#include <fhgcom/peer_info.hpp>

#include <fhg/util/thread/event.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <deque>
#include <exception>
#include <list>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>

namespace fhg
{
  namespace com
  {
    using Certificates = gspc::Certificates;

    /*!
      This class abstracts from an endpoint
     */
    class peer_t : private boost::noncopyable
    {
    public:
      typedef std::function <void (boost::system::error_code const &)> handler_t;

      peer_t ( std::unique_ptr<boost::asio::io_service>
             , host_t const& host
             , port_t const& port
             , Certificates const& certificates
             );

      virtual ~peer_t ();

      p2p::address_t const & address () const { return my_addr_.get(); }
      boost::asio::ip::tcp::endpoint local_endpoint() const
      {
        return acceptor_.local_endpoint();
      }

      p2p::address_t connect_to (host_t const&, port_t const&);

      void async_send ( p2p::address_t const& addr
                      , std::string const & data
                      , handler_t h
                      );
      void send ( p2p::address_t const& addr
                , std::string const & data
                );

      void async_recv
        ( std::function<void ( boost::system::error_code
                             , boost::optional<fhg::com::p2p::address_t> source
                             , message_t
                             )
                       >
        );

      std::exception_ptr TESTING_ONLY_handshake_exception() const;

    protected:
      void handle_hello_message (connection_t::ptr_t, std::unique_ptr<message_t> m);
      void handle_user_data   (connection_t::ptr_t, std::unique_ptr<message_t> m);
      void handle_error       (connection_t::ptr_t, const boost::system::error_code & error);

    private:
      struct to_send_t
      {
        to_send_t ()
          : message ()
          , handler()
        {}

        // \note Only valid until an async request was started,
        // i.e. before passing to connection::async_send().
        // Afterwards, only handler remains valid.
        // \todo Also move handler into connection::async_send to not
        // require split state.
        message_t  message;
        handler_t  handler;
      };

      using to_recv_t
        = std::function<void ( boost::system::error_code
                             , boost::optional<p2p::address_t>
                             , message_t
                             )
                       >;

      struct connection_data_t
      {
        connection_data_t ()
          : send_in_progress (false)
        {}

        bool send_in_progress;
        connection_t::ptr_t connection;
        std::deque<to_send_t> o_queue;
      };

      typedef std::recursive_mutex mutex_type;
      typedef std::unique_lock<mutex_type> lock_type;

      void accept_new ();
      void handle_accept (const boost::system::error_code &);
      void connection_established (lock_type const&, connection_data_t&);
      void handle_send (const p2p::address_t, const boost::system::error_code &);
      void start_sender (lock_type const&, connection_data_t&);

      friend class connection_t;

      void request_handshake_response
        ( p2p::address_t addr
        , std::shared_ptr<util::thread::event<std::exception_ptr>> connect_done
        , boost::system::error_code const& ec
        );
      void acknowledge_handshake_response
        (connection_t::ptr_t connection, boost::system::error_code const& ec);

      mutable mutex_type mutex_;

      bool stopping_;
      std::string host_;
      std::string port_;
      boost::optional<p2p::address_t> my_addr_;

      std::unique_ptr<boost::asio::ssl::context> ctx_;

      std::unique_ptr<boost::asio::io_service> io_service_;
      boost::asio::io_service::strand strand_;
      boost::asio::io_service::work io_service_work_;
      boost::asio::ip::tcp::acceptor acceptor_;

      std::unordered_map<p2p::address_t, connection_data_t> connections_;

      std::set<connection_t::ptr_t> backlog_;

      connection_t::ptr_t listen_;

      std::list<to_recv_t> m_to_recv;
      std::list<message_t> m_pending;

      std::exception_ptr TESTING_ONLY_handshake_exception_;
      boost::strict_scoped_thread<> _io_thread;
    };
  }
}
