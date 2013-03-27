#ifndef FHG_COM_SESSION_HPP
#define FHG_COM_SESSION_HPP 1

#include <fhglog/fhglog.hpp>

#include <vector>

#include <fhgcom/basic_session.hpp>
#include <fhgcom/basic_session_manager.hpp>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

// TODO: check if this conflicts with std::tr1::shared_ptr
#include <boost/enable_shared_from_this.hpp>

namespace fhg
{
  namespace com
  {
    class session : public basic_session
                  , public boost::enable_shared_from_this<session>
    {
    public:
      typedef basic_session_manager<session> manager_t;

      session ( boost::asio::io_service & io_service
              , manager_t & manager
              )
        : socket_(io_service)
        , manager_(manager)
        , stopped_(false)
      {}

      boost::asio::ip::tcp::socket & socket ()
      {
        return socket_;
      }

      ~session ()
      {
      }

      boost::asio::ip::tcp::endpoint remote_endpoint () const
      {
        return socket_.remote_endpoint();
      }

      boost::asio::ip::tcp::endpoint local_endpoint () const
      {
        return socket_.local_endpoint();
      }

      std::string remote_endpoint_str () const
      {
        return boost::lexical_cast<std::string>(remote_endpoint());
      }

      std::string local_endpoint_str () const
      {
        return boost::lexical_cast<std::string>(local_endpoint());
      }

      void start ()
      {
        manager_.add_session (shared_from_this());
        read_header ();
      }

      void close ();

      void async_send (const std::string & data);

    private:
      void read_header ();

      void handle_read_header ( const boost::system::error_code & error
                              , size_t bytes_recv
                              );

      void handle_read_data ( const boost::system::error_code & error
                            , size_t bytes_recv
                            );

      void handle_write (const boost::system::error_code &);

      boost::asio::ip::tcp::socket socket_;

      enum { header_length = 8 }; // 8 hex chars -> 4 bytes

      char inbound_header_[header_length];
      std::vector<char> inbound_data_;

      //      bool send_in_progress_;
      std::list<std::string> to_send_;

      manager_t & manager_;

      bool stopped_;
    };
  }
}

#endif
