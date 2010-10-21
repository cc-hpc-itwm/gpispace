#include "connection.hpp"
#include <fhglog/macros.hpp>
#include <fhgcom/util/to_hex.hpp>
#include <sstream>
#include <cassert>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

using boost::lambda::bind;
using boost::lambda::var;

namespace sdpa
{
  namespace com
  {
    connection::connection ( boost::asio::io_service & io_service
                           , std::string const & local_name
                           , std::string const & remote_name
                           )
      : io_service_(io_service)
      , socket_(io_service)
      , deadline_(io_service)
      , local_(local_name)
      , remote_(remote_name)
    {
      deadline_.expires_at(boost::posix_time::pos_infin);
      // check_deadline();
    }

    connection::~connection ()
    {

    }

    void connection::send ( std::string const & data
                          , boost::posix_time::time_duration timeout
                          )
    {
      deadline_.expires_from_now (timeout);
      boost::system::error_code ec = boost::asio::error::would_block;

      std::ostringstream sstr;
      sstr << std::setw(header_length)
           << std::setfill('0')
           << std::hex
           << data.size()
           << data
        ;

      DLOG( TRACE
          , "initiating write of "
          << fhg::com::util::basic_hex_converter<64>::convert( sstr.str().begin()
                                                             , sstr.str().end()
                                                             )
          );

      // Start the asynchronous operation itself. The boost::lambda function
      // object is used as a callback and will update the ec variable when the
      // operation completes. The blocking_udp_client.cpp example shows how you
      // can use boost::bind rather than boost::lambda.
      boost::asio::async_write( socket_
                              , boost::asio::buffer(sstr.str())
                              , var(ec) = boost::lambda::_1
                              );

      // Block until the asynchronous operation has completed.
      do io_service_.run_one(); while (ec == boost::asio::error::would_block);

      if (ec)
        throw boost::system::system_error(ec);
    }

    void connection::connect ( std::string const & host
                             , std::string const & port
                             , boost::posix_time::time_duration timeout
                             )
    {
      using namespace boost::asio::ip::tcp;

      tcp::resolver::query query(host, port);
      tcp::resolver::iterator iter = tcp::resolver(io_service_).resolve(query);

      // Set a deadline for the asynchronous operation. The host name may resolve
      // to multiple endpoints, and this function tries to connect to each one in
      // turn. Setting the deadline here means it applies to the entire sequence.
      deadline_.expires_from_now(timeout);

      boost::system::error_code ec;

      for (; iter != tcp::resolver::iterator(); ++iter)
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

        // Determine whether a connection was successfully established. The
        // deadline actor may have had a chance to run and close our socket, even
        // though the connect operation notionally succeeded. Therefore we must
        // check whether the socket is still open before deciding that the we
        // were successful.
        if (!ec && socket_.is_open())
        {
          LOG(INFO, "connected to " << socket_.remote_endpoint());
          return;
        }
      }

      throw boost::system::system_error
        (ec ? ec : boost::asio::error::host_not_found);
    }

    // ************************************************************** //
    peer::peer ( std::string const & name
               , std::string const & host
               , std::string const & port
               , connection_listener * listener
               , std::string const & kvs_host
               , std::string const & kvs_port = "2439"
               )
      : name_(name)
      , host_(host)
      , port_(port)
      , listener_(listener)
      , acceptor_(io_service_)
      , deadline_(io_service_)
    {
      try
      {
        kvs_.start(kvs_host, kvs_port);
      }
      catch (std::exception const & ex)
      {
        LOG(ERROR, "could not start the KVS backend: " << ex.what());
        throw();
      }
    }

    peer::~peer ()
    {
    }

    void peer::start ()
    {
      // start acceptor
    }

    void peer::stop ()
    {
      // stop acceptor
      // close open connections
    }

    void send ( const std::string & to
              , const std::string & data
              , boost::posix_time::time_duration timeout
              )
    {
      boost::unique_lock<boost::recursive_mutex> lock(mtx_);
      connection * c (NULL);
      if (connections_.find (to) != connections_.end())
      {
        c = connections_.at (to);
      }
      else
      {
        try
        {
          // lookup endpoint
          fhg::com::kvs::message::list::map_type
            location_data (kvs_.get ("net.location." + to));
          std::string h (location_data["host"]);
          std::string p (location_data["port"]);

          // connect
          c = new connection (io_service_, name_, to);
        }
        catch (std::exception const & ex)
        {
          LOG( ERROR
             , "could not send data: could not get location information for '"
             << to
             << "': "
             << ex.what()
             );
          throw;
        }
      }

      assert (c != NULL);
    }
  }
}
