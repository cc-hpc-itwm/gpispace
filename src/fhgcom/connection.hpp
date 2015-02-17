#pragma once

#include <fhgcom/header.hpp>
#include <fhgcom/message.hpp>

#include <fhg/assert.hpp>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>

#include <functional>
#include <iomanip>
#include <ios>
#include <list>
#include <stdint.h>
#include <string>
#include <vector>

namespace fhg
{
  namespace com
  {
    class connection_t : private boost::noncopyable
                       , public boost::enable_shared_from_this<connection_t>
    {
    public:
      typedef boost::shared_ptr<connection_t> ptr_t;

      typedef std::function <void (boost::system::error_code const &)> completion_handler_t;

      explicit
      connection_t
        ( boost::asio::io_service & io_service
        , std::function<void (ptr_t connection, const message_t*)> handle_hello_message
        , std::function<void (ptr_t connection, const message_t*)> handle_user_data
        , std::function<void (ptr_t connection, const boost::system::error_code&)> handle_error
        );

      ~connection_t ();

      boost::asio::ip::tcp::socket & socket ();

      void async_send (const message_t * msg, completion_handler_t hdl);

      template <typename SettableSocketOption>
      void set_option(const SettableSocketOption & o)
      {
        socket_.set_option (o);
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
    private:
      struct to_send_t
      {
        to_send_t (const message_t * msg, completion_handler_t hdl)
          : message(msg)
          , handler(hdl)
        {}
        const message_t * message;
        completion_handler_t handler;

        std::vector<boost::asio::const_buffer> const & to_buffers() const
        {
          fhg_assert (message != nullptr);

          if (message->data.size () != message->header.length)
          {
            throw std::length_error ("header/data length mismatch");
          }

          fhg_assert (message->data.size() == message->header.length);

          if (m_buf.empty ())
          {
            m_buf.push_back (boost::asio::buffer ( &message->header
                                                 , sizeof(p2p::header_t)
                                                 )
                            );
            m_buf.push_back (boost::asio::buffer (message->data));
          }

          return m_buf;
        }

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

      boost::asio::io_service::strand strand_;
      boost::asio::ip::tcp::socket socket_;

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
