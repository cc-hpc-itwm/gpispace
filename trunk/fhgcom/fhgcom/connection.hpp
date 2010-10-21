#ifndef FHGCOM_CONNECTION_HPP
#define FHGCOM_CONNECTION_HPP 1

#include <vector>
#include <string>
#include <stdint.h>

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/system/error_code.hpp>

#include <fhgcom/message.hpp>

namespace fhg
{
  namespace com
  {
    class message_handler_t
    {
    public:
      virtual ~message_handler_t () {}
      virtual void handle_recv ( const message_t * ) = 0;
      virtual void handle_error( const boost::system::error_code & error ) = 0;
    };

    typedef void (*completion_handler_t)(const boost::system::error_code & error);

    class connection_t : public boost::enable_shared_from_this<connection_t>
                       , private boost::noncopyable
    {
    private:
      typedef connection_t self;

    public:
      explicit
      connection_t ( boost::asio::io_service & io_service
                   , message_handler_t & message_handler
                   );

      ~connection_t ();

      boost::asio::ip::tcp::socket & socket ();

      void async_write ( const message_t * msg
                       , completion_handler_t hdl
                       , boost::posix_time::time_duration timeout = boost::posix_time::pos_infin
                       );

      void start ();
      void stop ();
    private:
      enum { header_length = 8 }; // 8 hex chars -> 4 bytes

      struct in_data_t
      {
        in_data_t ()
          : msg (0)
        {
          memset (header, 0, header_length);
        }
        char header [ header_length ];
        message_t * msg;
      };

      struct out_data_t
      {
        char header [ header_length ];
        message_t * msg;
        completion_handler_t hdl;
        boost::posix_time::time_duration timeout;
      };

      void start_read ();
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
      message_handler_t & message_handler_;
      in_data_t in_data_;
      std::vector <out_data_t> to_send_;
    };
  }
}

#endif
