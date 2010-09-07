#include <fhglog/macros.hpp>
#include <fhgcom/util/to_hex.hpp>

#include "tcp_client.hpp"

namespace fhg
{
  namespace com
  {
    tcp_client::tcp_client ( boost::asio::io_service & io_service
                           , const std::string & host
                           , const std::string & service
                           )
      : socket_(io_service)
      , stopped_(false)
    {
      boost::asio::ip::tcp::resolver resolver (io_service);
      boost::asio::ip::tcp::resolver::query query(host, service);
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator
        (resolver.resolve (query));
      boost::asio::ip::tcp::endpoint endpoint (*endpoint_iterator);

      DLOG (TRACE, "trying to connect to " << endpoint);
      socket_.async_connect( endpoint
                           , boost::bind( &tcp_client::handle_connect
                                        , this
                                        , boost::asio::placeholders::error
                                        , ++endpoint_iterator
                                        )
                           );
    }

    tcp_client::~tcp_client ()
    {
      try
      {
        close ();
      }
      catch (...)
      {
        LOG(ERROR, "error in destructor of tcp_client, could not close correctly");
      }
    }

    void tcp_client::close ()
    {
      lock_t lock (mutex_);

      stopped_ = true;

      while (to_send_.size())
        data_sent_.wait(lock);

      if (to_recv_.size())
      {
        LOG(WARN, "there was still data to be received!");
        to_recv_.clear();
      }

      socket_.close();
      DLOG(TRACE, "session closed");
    }

    void tcp_client::send ( const std::string & data )
    {
      if (stopped_)
        throw std::runtime_error ("not connected");

      std::ostringstream sstr;
      sstr << std::setw(header_length)
           << std::setfill('0')
           << std::hex
           << data.size()
           << data
        ;

      bool send_in_progress (false);
      {
        lock_t lock (mutex_);
        send_in_progress = !to_send_.empty();
        to_send_.push_back (sstr.str());
      }

      if (!send_in_progress)
      {
        DLOG(TRACE, "initiating write of " << to_send_.front());

        boost::asio::async_write( socket_
                                , boost::asio::buffer( to_send_.front().data()
                                                     , to_send_.front().length()
                                                     )
                                , boost::bind( &tcp_client::handle_write
                                             , this
                                             , boost::asio::placeholders::error
                                             )
                                );
      }
    }

    std::string tcp_client::recv ()
    {
      if (stopped_)
        throw std::runtime_error ("stopped");

      lock_t lock (mutex_);
      while (to_recv_.empty())
      {
        data_rcvd_.wait (lock);

        if (stopped_)
          throw std::runtime_error ("stopped");
      }

      std::string data (to_recv_.front()); to_recv_.pop_front();

      if (! to_recv_.empty())
      {
        data_rcvd_.notify_one();
      }

      return data;
    }

    void tcp_client::handle_connect ( const boost::system::error_code & e
                                    , boost::asio::ip::tcp::resolver::iterator endpoint_iterator
                                    )
    {
      if (stopped_)
        return;

      if (! e)
      {
        DLOG (TRACE, "connected");
        read_header();
      }
      else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
      {
        socket_.close();
        boost::asio::ip::tcp::endpoint endpoint (*endpoint_iterator);

        DLOG (TRACE, "trying to connect to " << endpoint);
        socket_.async_connect( endpoint
                             , boost::bind( &tcp_client::handle_connect
                                          , this
                                          , boost::asio::placeholders::error
                                          , ++endpoint_iterator
                                          )
                             );
      }
      else
      {
        LOG(ERROR, "could not connect: " << e.message() << ": " << e);
        close ();
      }
    }

    void tcp_client::handle_write (const boost::system::error_code & e)
    {
      if (stopped_)
        return;

      if (! e)
      {
        DLOG(TRACE, "write completed");

        bool more_to_send (false);
        {
          lock_t lock (mutex_);
          to_send_.pop_front();

          data_sent_.notify_one();

          more_to_send = ! to_send_.empty();
        }

        if (more_to_send)
        {
          boost::asio::async_write( socket_
                                  , boost::asio::buffer( to_send_.front().data()
                                                       , to_send_.front().length()
                                                       )
                                  , boost::bind( &tcp_client::handle_write
                                               , this
                                               , boost::asio::placeholders::error
                                               )
                                  );
        }
      }
      else
      {
        LOG(ERROR, "could not send data: " << e << ": " << e.message());
        close ();
      }
    }

    void tcp_client::read_header ()
    {
      DLOG(DEBUG, "trying to receive header of length " << header_length);
      boost::asio::async_read ( socket_
                              , boost::asio::buffer (inbound_header_)
                              , boost::bind ( &tcp_client::handle_read_header
                                            , this
                                            , boost::asio::placeholders::error
                                            , boost::asio::placeholders::bytes_transferred
                                            )
                              );
    }

    void tcp_client::handle_read_header ( const boost::system::error_code & error
                                        , size_t bytes_recv
                                        )
    {
      if (stopped_)
        return;

      if (!error)
      {
        DLOG(DEBUG, "received " << bytes_recv << " bytes of header");
        std::istringstream is (std::string (inbound_header_, header_length));
        std::size_t inbound_data_size = 0;
        if (! (is >> std::hex >> inbound_data_size))
        {
          LOG(ERROR, "could not parse header: " << std::string(inbound_header_, header_length));
          // TODO: call handler
          return;
        }

        DLOG(DEBUG, "going to receive " << inbound_data_size << " bytes");

        inbound_data_.resize (inbound_data_size);

        boost::asio::async_read ( socket_
                                , boost::asio::buffer (inbound_data_)
                                , boost::bind ( &tcp_client::handle_read_data
                                              , this
                                              , boost::asio::placeholders::error
                                              , boost::asio::placeholders::bytes_transferred
                                              )
                                );
      }
      else
      {
        LOG(WARN, "session closed: " << error.message() << " := " << error);
      }
    }

    void tcp_client::handle_read_data ( const boost::system::error_code & error
                                      , size_t bytes_recv
                                      )
    {
      if (stopped_)
        return;

      if (!error)
      {
        DLOG(TRACE, "received " << bytes_recv << " bytes: "
            << util::basic_hex_converter<64>::convert( inbound_data_.begin()
                                                     , inbound_data_.end()
                                                     )
            );

        {
          lock_t lock (mutex_);
          to_recv_.push_back(std::string(&inbound_data_[0], inbound_data_.size()));
          data_rcvd_.notify_one();
        }

        read_header ();
      }
      else
      {
        LOG(ERROR, "session got error during chunk receive := " << error);
      }
    }
  }
}
