#ifndef FHG_COM_SESSION_HPP
#define FHG_COM_SESSION_HPP 1

#include <fhglog/fhglog.hpp>

#include <vector>

#include <fhgcom/util/to_hex.hpp>
#include <fhgcom/basic_session.hpp>
#include <fhgcom/session_manager.hpp>

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
      typedef session_manager manager_t;

      session ( boost::asio::io_service & io_service
              , manager_t & manager
              )
        : socket_(io_service)
        , manager_(manager)
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
        manager_.add (shared_from_this());
        read_header ();
      }

    private:
      void read_header ()
      {
        DLOG(DEBUG, "trying to receive header of length " << header_length);
        boost::asio::async_read ( socket_
                                , boost::asio::buffer (inbound_header_)
                                , boost::bind ( &session::handle_read_header
                                              , shared_from_this()
                                              , boost::asio::placeholders::error
                                              , boost::asio::placeholders::bytes_transferred
                                              )
                                );
      }

      void handle_read_header ( const boost::system::error_code & error
                              , size_t bytes_recv
                              )
      {
        if (!error)
        {
          DLOG(DEBUG, "received " << bytes_recv << " bytes of header");
          std::istringstream is (std::string (inbound_header_, header_length));
          std::size_t inbound_data_size = 0;
          if (! (is >> std::hex >> inbound_data_size))
          {
            LOG(ERROR, "could not parse header: " << std::string(inbound_header_, header_length));
            manager_.del (shared_from_this());
            // TODO: call handler
            return;
          }

          DLOG(DEBUG, "going to receive " << inbound_data_size << " bytes");

          inbound_data_.resize (inbound_data_size);

          boost::asio::async_read ( socket_
                                  , boost::asio::buffer (inbound_data_)
                                  , boost::bind ( &session::handle_read_data
                                                , shared_from_this()
                                                , boost::asio::placeholders::error
                                                , boost::asio::placeholders::bytes_transferred
                                                )
                                  );
        }
        else
        {
          LOG(WARN, "session closed: " << error.message() << ": " << error);
          manager_.del (shared_from_this());
        }
      }

      void handle_read_data ( const boost::system::error_code & error
                            , size_t bytes_recv
                            )
      {
        if (!error)
        {
          std::string data(&inbound_data_[0], inbound_data_.size());
          DLOG(TRACE, "received " << bytes_recv << " bytes: " << util::to_hex (data));

          read_header ();
        }
        else
        {
          LOG(ERROR, "session got error during read: " << error.message() << ": " << error);
          manager_.del (shared_from_this());
        }
      }

      boost::asio::ip::tcp::socket socket_;

      enum { header_length = 8 }; // 8 hex chars -> 4 bytes

      std::string outbound_header_;
      std::string outbound_data_;

      char inbound_header_[header_length];
      std::vector<char> inbound_data_;

      manager_t & manager_;
    };
  }
}

#endif
