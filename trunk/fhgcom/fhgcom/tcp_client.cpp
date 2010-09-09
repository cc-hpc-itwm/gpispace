#include <fhglog/macros.hpp>
#include <fhgcom/util/to_hex.hpp>

#include "tcp_client.hpp"

using boost::asio::ip::tcp;

namespace fhg
{
  namespace com
  {
    tcp_client::tcp_client ( boost::asio::io_service & io_service
                           , const std::string & host
                           , const std::string & port
                           )
      : io_service_(io_service)
      , socket_(io_service)
      , host_(host)
      , port_(port)
      , send_in_progress_(false)
      , stopped_(false)
      , connected_(false)
      , deadline_(io_service)
    {
    }

    tcp_client::~tcp_client ()
    {
      try
      {
        stop ();
      }
      catch (...)
      {
        LOG(ERROR, "error in destructor of tcp_client, could not close correctly");
      }
    }

    void tcp_client::start ()
    {
      start (host_, port_);
    }

    void tcp_client::start ( const std::string & host
                           , const std::string & port
                           )
    {
      host_ = host;
      port_ = port;

      tcp::resolver resolver (io_service_);
      tcp::resolver::query query(host, port);

      start (resolver.resolve (query));
    }

    void tcp_client::stop ()
    {
      lock_t lock (mutex_);

      if (stopped_) return;

      stopped_ = true;
      connected_ = false;
      send_in_progress_ = false;

      socket_.close();

      if (to_send_.size())
      {
        LOG(WARN, "there was still data to send!");
        to_send_.clear();
      }

      if (to_recv_.size())
      {
        LOG(WARN, "there was still data to be received!");
        to_recv_.clear();
      }

      // wake up receivers
      data_rcvd_.notify_all ();

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

      {
        lock_t lock (mutex_);
        to_send_.push_back (sstr.str());
      }

      start_writer();
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


    void tcp_client::start(tcp::resolver::iterator endpoint_iter)
    {
      start_connect (endpoint_iter);

      deadline_.async_wait (boost::bind(&tcp_client::check_deadline, this));
    }


    void tcp_client::start_connect (tcp::resolver::iterator endpoint_iter)
    {
      if (endpoint_iter != tcp::resolver::iterator())
      {
        DLOG (TRACE, "trying to connect to " << endpoint_iter->endpoint());
        deadline_.expires_from_now(boost::posix_time::seconds(30));

        socket_.async_connect( endpoint_iter->endpoint()
                             , boost::bind( &tcp_client::handle_connect
                                          , this
                                          , _1
                                          , endpoint_iter
                                          )
                             );
      }
      else
      {
        stop();
      }
    }

    void tcp_client::start_reader ()
    {
      read_header ();
    }

    void tcp_client::start_writer ()
    {
      if (stopped_)
        return;

      lock_t lock (mutex_);
      if (!send_in_progress_ && !to_send_.empty() && connected_)
      {
        DLOG( TRACE
            , "initiating write of "
            << util::basic_hex_converter<64>::convert( to_send_.front().begin()
                                                     , to_send_.front().end()
                                                     )
            );
        send_in_progress_ = true;
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

    void tcp_client::check_deadline ()
    {
      if (stopped_)
        return;

      if (deadline_.expires_at() <= boost::asio::deadline_timer::traits_type::now())
      {
        socket_.close();
        deadline_.expires_at(boost::posix_time::pos_infin);
      }

      deadline_.async_wait(boost::bind(&tcp_client::check_deadline, this));
    }

    void tcp_client::handle_connect ( const boost::system::error_code & e
                                    , boost::asio::ip::tcp::resolver::iterator endpoint_iter
                                    )
    {
      if (stopped_)
        return;

      if (!socket_.is_open())
      {
        LOG(WARN, "connection attempt timed out!");
        start_connect (++endpoint_iter);
      }
      else if (e)
      {
        LOG(WARN, "connection attempt failed: " << e.message());

        socket_.close();
        start_connect (++endpoint_iter);
      }
      else
      {
        DLOG (TRACE, "successfully connected to: " << endpoint_iter->endpoint());

        connected_ = true;

        start_reader ();
        start_writer ();
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
        else
        {
          send_in_progress_ = false;
        }
      }
      else
      {
        LOG(ERROR, "could not send data: " << e << ": " << e.message());
        stop ();
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
        stop ();
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
        stop ();
      }
    }
  }
}
