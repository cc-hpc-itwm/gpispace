#include <fhglog/macros.hpp>
#include <fhgcom/util/to_hex.hpp>

#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "tcp_client.hpp"

using boost::asio::deadline_timer;
using boost::asio::ip::tcp;
using boost::lambda::bind;
using boost::lambda::var;
//using boost::lambda::_1;

namespace fhg
{
  namespace com
  {
    tcp_client::tcp_client ()
      : socket_(io_service_)
      , deadline_(io_service_)
      , auto_reconnect_ (false)
      , connection_attempts_(0)
      , max_connection_attempts_(0)
    {
      deadline_.expires_at (boost::posix_time::pos_infin);

      check_deadline ();
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

    void tcp_client::start ( const std::string & host
                           , const std::string & port
                           , bool auto_reconnect
                           , boost::posix_time::time_duration timeout
                           , const std::size_t max_connection_attempts
                           )
    {
      host_ = host;
      port_ = port;
      auto_reconnect_ = auto_reconnect;
      connection_attempts_ = 0;
      max_connection_attempts_ = max_connection_attempts;
      timeout_ = timeout;

      reconnect();
    }

    void tcp_client::reconnect ()
    {
      connection_attempts_ = 0;

      for (;;)
      {
        try
        {
          connect ();
          break;
        }
        catch (std::exception const & ex)
        {
          ++connection_attempts_;

          LOG( TRACE
             , "connecting to [" << host_ << "]:" << port_
             << " failed: " << ex.what()
             << " attempt: " << (connection_attempts_) << "/" << max_connection_attempts_
             );

          if (max_connection_attempts_ && connection_attempts_ >= max_connection_attempts_)
          {
            throw;
          }
          else
          {
            // TODO: this is probably bad, but we have to wait until we reconnect
            boost::posix_time::time_duration reconnect_in
              (boost::posix_time::microseconds
              ((int)std::ceil ((float)timeout_.total_microseconds() / (float)(std::max(max_connection_attempts_, std::size_t(2)) - 1)))
              );
            LOG(TRACE, "reconnecting in " << reconnect_in);
            boost::this_thread::sleep(boost::posix_time::microseconds(reconnect_in.total_microseconds()));
          }
        }
      }
    }

    void tcp_client::connect ()
    {
      // Resolve the host name and service to a list of endpoints.
      tcp::resolver::query query(host_, port_);
      tcp::resolver::iterator iter = tcp::resolver(io_service_).resolve(query);

      // Set a deadline for the asynchronous operation. The host name may resolve
      // to multiple endpoints, and this function tries to connect to each one in
      // turn. Setting the deadline here means it applies to the entire sequence.
      deadline_.expires_from_now(timeout_);

      boost::system::error_code ec;

      for (; iter != tcp::resolver::iterator(); ++iter)
      {
        do
        {
          // We may have an open socket from a previous connection attempt. This
          // socket cannot be reused, so we must close it before trying to connect
          // again.
          socket_.close();

          // Set up the variable that receives the result of the asynchronous
          // operation. The error code is set to would_block to signal that the
          // operation is incomplete. Asio guarantees that its asynchronous
          // operations will never fail with would_block, so any other value in
          // ec indicates completion.
          ec = boost::asio::error::would_block;

          DLOG(TRACE, "trying to connect to " << iter->endpoint());

          // Start the asynchronous operation itself. The boost::lambda function
          // object is used as a callback and will update the ec variable when the
          // operation completes. The blocking_udp_client.cpp example shows how you
          // can use boost::bind rather than boost::lambda.
          socket_.async_connect(iter->endpoint(), var(ec) = boost::lambda::_1);

          // Block until the asynchronous operation has completed.
          do io_service_.run_one(); while (ec == boost::asio::error::would_block);

        } while (ec == boost::system::errc::address_not_available);

        // Determine whether a connection was successfully established. The
        // deadline actor may have had a chance to run and close our socket, even
        // though the connect operation notionally succeeded. Therefore we must
        // check whether the socket is still open before deciding that the we
        // were successful.
        if (!ec && socket_.is_open())
        {
          MLOG (TRACE, "connected to " << socket_.remote_endpoint());
          return;
        }
      }

      throw boost::system::system_error
        (ec ? ec : boost::asio::error::host_not_found);
    }

    void tcp_client::stop ()
    {
      socket_.close();
    }

    std::string tcp_client::request ( const std::string & reqst
                                    , boost::posix_time::time_duration timeout
                                    )
    {
      for (;;)
      {
        try
        {
          try_send (reqst, timeout);
          return try_recv (timeout);
        }
        catch (std::exception const &ex)
        {
          if (auto_reconnect_)
          {
            reconnect ();
          }
          else
          {
            throw;
          }
        }
      }
    }

    void tcp_client::send ( const std::string & data
                          , boost::posix_time::time_duration timeout
                          )
    {
      for (;;)
      {
        try
        {
          return try_send ( data, timeout );
        }
        catch (...)
        {
          if (auto_reconnect_)
          {
            reconnect ();
          }
        }
      }
    }

    void tcp_client::try_send ( const std::string & data
                              , boost::posix_time::time_duration timeout
                              )
    {
      if (timeout.total_microseconds () > 0)
        deadline_.expires_from_now (timeout);

      // Set up the variable that receives the result of the asynchronous
      // operation. The error code is set to would_block to signal that the
      // operation is incomplete. Asio guarantees that its asynchronous
      // operations will never fail with would_block, so any other value in
      // ec indicates completion.
      boost::system::error_code ec = boost::asio::error::would_block;

      std::ostringstream sstr;
      sstr << std::setw(header_length)
           << std::setfill('0')
           << std::hex
           << data.size()
           << data
        ;

      // Start the asynchronous operation itself. The boost::lambda function
      // object is used as a callback and will update the ec variable when the
      // operation completes. The blocking_udp_client.cpp example shows how you
      // can use boost::bind rather than boost::lambda.
      boost::asio::async_write(socket_, boost::asio::buffer(sstr.str()), var(ec) = boost::lambda::_1);

      // Block until the asynchronous operation has completed.
      do io_service_.run_one(); while (ec == boost::asio::error::would_block);

      if (ec)
      {
        LOG(WARN, "send failed: " << ec);
        throw boost::system::system_error(ec);
      }
    }

    std::string tcp_client::recv (boost::posix_time::time_duration timeout)
    {
      for (;;)
      {
        try
        {
          return try_recv (timeout);
        }
        catch (...)
        {
          if (auto_reconnect_)
          {
            reconnect ();
          }
        }
      }
    }

    std::string tcp_client::try_recv (boost::posix_time::time_duration timeout)
    {
      if (timeout.total_microseconds () > 0)
        deadline_.expires_from_now (timeout);

      // Set up the variable that receives the result of the asynchronous
      // operation. The error code is set to would_block to signal that the
      // operation is incomplete. Asio guarantees that its asynchronous
      // operations will never fail with would_block, so any other value in
      // ec indicates completion.
      boost::system::error_code ec = boost::asio::error::would_block;

      char header[header_length];
      boost::asio::async_read ( socket_
                              , boost::asio::buffer (header, header_length)
                              , var(ec) = boost::lambda::_1
                              );

      // Block until the asynchronous operation has completed.
      do io_service_.run_one(); while (ec == boost::asio::error::would_block);

      if (ec)
      {
        DMLOG_IF( WARN
                , ec.value() != boost::asio::error::eof
                , "recv failed: " << ec << ": " << ec.message();
                );
        throw boost::system::system_error ( ec.value()
                                          , boost::asio::error::get_misc_category()
                                          );
      }

      std::size_t data_size = 0;
      {
        std::istringstream is (std::string (header, header_length));
        if (! (is >> std::hex >> data_size))
        {
          DMLOG (ERROR, "could not parse header: " << std::string(header, header_length));
          throw std::runtime_error ("could not parse header!");
        }
      }

      std::vector<char> data;
      data.resize(data_size);

      ec = boost::asio::error::would_block;
      boost::asio::async_read ( socket_
                              , boost::asio::buffer (data)
                              , var(ec) = boost::lambda::_1
                              );

      // Block until the asynchronous operation has completed.
      do io_service_.run_one(); while (ec == boost::asio::error::would_block);

      if (ec)
      {
        DMLOG_IF( WARN
                , ec.value() != boost::asio::error::eof
                , "recv failed: " << ec << ": " << ec.message();
                );
        throw boost::system::system_error ( ec.value()
                                          , boost::asio::error::get_system_category()
                                          );
      }

      return std::string (&data[0], data.size());
    }

    void tcp_client::check_deadline ()
    {
      // Check whether the deadline has passed. We compare the deadline against
      // the current time since a new asynchronous operation may have moved the
      // deadline before this actor had a chance to run.
      if (deadline_.expires_at() <= deadline_timer::traits_type::now())
      {
        // The deadline has passed. The socket is closed so that any outstanding
        // asynchronous operations are cancelled. This allows the blocked
        // connect(), read_line() or write_line() functions to return.
        socket_.close();

        // There is no longer an active deadline. The expiry is set to positive
        // infinity so that the actor takes no action until a new deadline is set.
        deadline_.expires_at(boost::posix_time::pos_infin);
      }

      // Put the actor back to sleep.
      deadline_.async_wait(bind(&tcp_client::check_deadline, this));
    }
  }
}
