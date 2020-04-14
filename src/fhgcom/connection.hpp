#pragma once

#include <fhgcom/header.hpp>
#include <fhgcom/message.hpp>

#include <fhg/assert.hpp>
#include <fhg/util/thread/event.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#include <boost/variant/variant.hpp>

#include <cstddef>
#include <exception>
#include <functional>
#include <list>
#include <memory>
#include <stdexcept>
#include <vector>

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
    class peer_t;

    using tcp_socket_t = boost::asio::ip::tcp::socket;
    using ssl_stream_t = boost::asio::ssl::stream<tcp_socket_t>;
    using socket_t = boost::variant<std::unique_ptr<tcp_socket_t>, std::unique_ptr<ssl_stream_t>>;

    class handshake_exception : public boost::system::system_error
    {
    public:
      handshake_exception (boost::system::error_code const& ec)
        : boost::system::system_error (ec)
      {}
    };

    class connection_t : private boost::noncopyable
                       , public boost::enable_shared_from_this<connection_t>
    {
    public:
      typedef boost::shared_ptr<connection_t> ptr_t;

      typedef std::function <void (boost::system::error_code const &)> completion_handler_t;

      explicit
      connection_t
        ( boost::asio::io_service & io_service
        , boost::asio::ssl::context* ctx
        , boost::asio::io_service::strand const& strand
        , std::function<void (ptr_t connection, const message_t*)> handle_hello_message
        , std::function<void (ptr_t connection, const message_t*)> handle_user_data
        , std::function<void (ptr_t connection, const boost::system::error_code&)> handle_error
        , peer_t* peer
        );

      ~connection_t ();

      boost::asio::ip::tcp::socket & socket();

      void async_send (const message_t * msg, completion_handler_t hdl);

      template <typename SettableSocketOption>
      void set_option(const SettableSocketOption & o)
      {
        socket().set_option (o);
      }

      void start ();
      void stop ();

      const p2p::address_t & local_address () const { return m_local_addr; }
      const p2p::address_t & remote_address () const { return m_remote_addr; }

      void local_address  (const p2p::address_t & a) { m_local_addr = a; }
      void remote_address (const p2p::address_t & a)
      {
        m_remote_addr = a;
      }

      //! Assumes to be called from within strand. Will call back
      //! peer_t::request_handshake_response via strand, never from
      //! within this call stack.
      void request_handshake
        (std::shared_ptr<util::thread::event<std::exception_ptr>> connect_done);
      //! Assumes to be called from within strand. Will call back
      //! peer_t::acknowledge_handshake_response via strand, never
      //! from within this call stack.
      void acknowledge_handshake();

    private:
      struct to_send_t
      {
        to_send_t (const message_t * msg, completion_handler_t hdl);
        const message_t * message;
        completion_handler_t handler;

        std::vector<boost::asio::const_buffer> const & to_buffers() const;

      private:
        mutable std::vector<boost::asio::const_buffer> m_buf;
      };

      void start_read ();
      void start_send ();
      void start_send (to_send_t const &);

      void handle_read_header ( const boost::system::error_code & ec
                              , std::size_t bytes_transferred
                              );
      void handle_read_data ( const boost::system::error_code & ec
                            , std::size_t bytes_transferred
                            );

      void handle_write ( const boost::system::error_code & ec );

      peer_t* _peer;

      boost::asio::io_service::strand strand_;
      socket_t socket_;
      tcp_socket_t& _raw_socket;
      std::function<void (ptr_t connection, const message_t*)> _handle_hello_message;
      std::function<void (ptr_t connection, const message_t*)> _handle_user_data;
      std::function<void (ptr_t connection, const boost::system::error_code&)> _handle_error;
      message_t *in_message_;

      std::list <to_send_t> to_send_;

      p2p::address_t m_local_addr;
      p2p::address_t m_remote_addr;
    };
  }
}
