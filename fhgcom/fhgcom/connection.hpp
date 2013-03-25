#ifndef FHGCOM_CONNECTION_HPP
#define FHGCOM_CONNECTION_HPP 1

#include <vector>
#include <string>
#include <stdint.h>
#include <iomanip>
#include <ios>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/system/error_code.hpp>

#include <fhg/assert.hpp>
#include <fhgcom/header.hpp>
#include <fhgcom/message.hpp>

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

      typedef boost::function <void (boost::system::error_code const &)> completion_handler_t;
      //      typedef void (*completion_handler_t)(const boost::system::error_code & error);

      explicit
      connection_t ( boost::asio::io_service & io_service
                   , std::string const & cookie
                   , handler_t * h
                   );

      ~connection_t ();

      handler_t * set_callback_handler ( handler_t * h );

      ptr_t get_this ()
      {
        return shared_from_this();
      }

      boost::asio::ip::tcp::socket & socket ();

      template <typename Handler>
      void async_send ( const message_t * msg
                      , Handler hdl
                      , boost::posix_time::time_duration timeout = boost::posix_time::pos_infin
                      );

      template <typename SettableSocketOption>
      void set_option(const SettableSocketOption & o)
      {
        socket_.set_option (o);
      }

      template <typename GettableSocketOption>
      void get_option(GettableSocketOption & o) const
      {
        socket_.get_option (o);
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
                                                 , sizeof(message_t::header_t)
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

      std::string cookie_;
      handler_t *callback_handler_;
      message_t *in_message_;

      std::list <to_send_t> to_send_;

      p2p::address_t m_local_addr;
      p2p::address_t m_remote_addr;
    };

    template <typename Handler>
    void connection_t::async_send ( const message_t * msg
                                  , Handler hdl
                                  , boost::posix_time::time_duration timeout
                                  )
    {
      strand_.post (boost::bind( &self::start_send
                               , get_this()
                               , to_send_t (msg, hdl, timeout)
                               ));
    }
  }
}

#endif
