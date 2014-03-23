#ifndef FHGCOM_CONNECTION_HPP
#define FHGCOM_CONNECTION_HPP 1

#include <fhgcom/header.hpp>
#include <fhgcom/message.hpp>

#include <fhg/assert.hpp>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
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
    private:
      typedef connection_t self;

    public:
      typedef boost::shared_ptr<connection_t> ptr_t;

      class handler_t
      {
      public:
        virtual ~handler_t () {}

        virtual void handle_system_data (ptr_t connection, const message_t *) = 0;
        virtual void handle_user_data   (ptr_t connection, const message_t *) = 0;
        virtual void handle_error       (ptr_t connection, const boost::system::error_code & error ) = 0;
      };

      typedef std::function <void (boost::system::error_code const &)> completion_handler_t;

      explicit
      connection_t ( boost::asio::io_service & io_service
                   , handler_t * h
                   );

      ~connection_t ();

      ptr_t get_this ()
      {
        return shared_from_this();
      }

      boost::asio::ip::tcp::socket & socket ();

      void async_send ( const message_t * msg
                      , completion_handler_t hdl
                      , boost::posix_time::time_duration timeout = boost::posix_time::pos_infin
                      );

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
      void remote_address (const p2p::address_t & a) { m_remote_addr = a; }
    private:
      struct to_send_t
      {
        to_send_t (const message_t * msg, completion_handler_t hdl, boost::posix_time::time_duration to)
          : message(msg)
          , handler(hdl)
          , timeout(to)
        {}
        const message_t * message;
        completion_handler_t handler;
        boost::posix_time::time_duration timeout;

        std::vector<boost::asio::const_buffer> const & to_buffers() const
        {
          assert (message != 0);

          if (message->data.size () != message->header.length)
          {
            throw std::length_error ("header/data length mismatch");
          }

          assert (message->data.size() == message->header.length);

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
      boost::asio::deadline_timer deadline_;

      handler_t *callback_handler_;
      message_t *in_message_;

      std::list <to_send_t> to_send_;

      p2p::address_t m_local_addr;
      p2p::address_t m_remote_addr;
    };
  }
}

#endif
