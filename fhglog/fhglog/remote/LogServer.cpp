#include <boost/bind.hpp>

#include <fhglog/fhglog.hpp>

#include "LogServer.hpp"

using namespace fhg::log::remote;
using boost::asio::ip::udp;

LogServer::LogServer(const fhg::log::Appender::ptr_t &appender
                   , boost::asio::io_service &io_service
                   , unsigned short port)
  : appender_(appender)
    //  , io_service_(io_service)
  , socket_(io_service, udp::endpoint(udp::v4(), port))
{
  LOG(INFO, "log server listening on " << udp::endpoint(udp::v4(), port));
  boost::system::error_code ec;

  socket_.set_option (boost::asio::socket_base::reuse_address (true), ec);
  LOG_IF(WARN, ec, "could not set resuse address option: " << ec << ": " << ec.message());

  socket_.async_receive_from(boost::asio::buffer(data_, max_length), sender_endpoint_,
      boost::bind(&LogServer::handle_receive_from, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));

}

void LogServer::handle_receive_from(const boost::system::error_code &error
                                  , size_t bytes_recv)
{
  if (!error && bytes_recv > 0)
  {
    data_[bytes_recv] = 0;
    std::string msg(data_);

    if (msg == "QUIT deadbeef")
    {
      LOG(INFO, "got QUIT message, shutting down.");
      return;
    }

    try
    {
      LogEvent evt;
      std::stringstream sstr (msg);
      evt.decode (sstr);

      {
        std::ostringstream ostr;
        ostr << sender_endpoint_;
        evt.trace (ostr.str());
      }
      appender_->append (evt);
    }
    catch (std::exception const &ex)
    {
      LOG (WARN, "error during log-event decode: " << ex.what ());
    }
  }
  else
  {
    LOG(ERROR, "error during receive: " << error);
  }

  socket_.async_receive_from( boost::asio::buffer(data_, max_length)
                            , sender_endpoint_
                            , boost::bind( &LogServer::handle_receive_from
                                         , this
                                         , boost::asio::placeholders::error
                                         , boost::asio::placeholders::bytes_transferred
                                         )
                            );
}
