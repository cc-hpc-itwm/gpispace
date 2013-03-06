#include "session.hpp"
#include <fhgcom/util/to_hex.hpp>
#include <boost/bind.hpp>

using namespace fhg::com;

void session::close ()
{
  stopped_ = true;
  manager_.del_session (shared_from_this());
  socket_.close ();
}

// TODO: make data a pointer to allocated  memory, so that we don't have to copy
// it!
void session::async_send (const std::string & data)
{
  // TODO: store two buffers, one for the header, one for the data
  std::ostringstream sstr;
  sstr << std::setw(header_length)
       << std::setfill('0')
       << std::hex
       << data.size()
       << data
    ;

  // TODO: create  id or  something to identify  this write and  call callback
  // later
  bool send_in_progress_ = !to_send_.empty();
  to_send_.push_back (sstr.str());
  if (!send_in_progress_)
  {
    DLOG(TRACE, "initiating write of " << data.length() << " bytes " << util::log_raw (data));

    boost::asio::async_write( socket_
                            , boost::asio::buffer( to_send_.front().data()
                                                 , to_send_.front().length()
                                                 )
                            , boost::bind( &session::handle_write
                                         , shared_from_this ()
                                         , boost::asio::placeholders::error
                                         )
                            );
  }
}

void session::read_header ()
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

void session::handle_read_header ( const boost::system::error_code & error
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
      LOG( ERROR
         , "could not parse header: " << std::string(inbound_header_, header_length)
         );
      close ();
      // TODO: call handler
      return;
    }

    DLOG(DEBUG, "going to receive " << inbound_data_size << " bytes");

    DLOG_IF( WARN
           , inbound_data_size > (1 << 27)
           , "incoming message is quite large!"
           );

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
    LOG_IF(WARN, error.value() != 2, "session failed: " << error.message() << ": " << error);
    close ();
  }
}

void session::handle_read_data ( const boost::system::error_code & error
                               , size_t /* bytes_recv */
                               )
{
  if (stopped_)
    return;

  if (!error)
  {
    try
    {
      const std::string data(&inbound_data_[0], inbound_data_.size());

      DLOG(TRACE, "received " << data.size() << " bytes " << util::log_raw (data));

      manager_.handle_data (shared_from_this(), data);
    }
    catch (std::exception const & ex)
    {
      LOG(ERROR, "Could not handle data: " << ex.what());
    }

    read_header ();
  }
  else
  {
    LOG( ERROR
       , "session got error during read: " << error.message() << ": " << error
       );
    close ();
  }
}


void session::handle_write (const boost::system::error_code & e)
{
  if (stopped_)
    return;

  if (! e)
  {
    DLOG(TRACE, "write completed");

    if (to_send_.empty())
    {
      LOG( ERROR
         , "*** INCONSISTENCY ****"
         << std::endl
         << "in session::handle_write - outgoing list should not be empty!"
         );
    }
    else
    {
      to_send_.pop_front();
    }

    if (! to_send_.empty())
    {
      boost::asio::async_write( socket_
                              , boost::asio::buffer( to_send_.front().data()
                                                   , to_send_.front().length()
                                                   )
                              , boost::bind( &session::handle_write
                                           , shared_from_this ()
                                           , boost::asio::placeholders::error
                                           )
                              );
    }
  }
  else
  {
    // TODO: close session, inform manager
    LOG(ERROR, "could not send data: " << e << ": " << e.message());
    close ();
  }
}
