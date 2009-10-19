/*
 * =====================================================================================
 *
 *       Filename:  LogServer.cpp
 *
 *    Description:  log server implementation
 *
 *        Version:  1.0
 *        Created:  10/19/2009 09:08:11 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <boost/bind.hpp>

#include <fhglog/fhglog.hpp>

#include "LogServer.hpp"

using namespace fhg::log::remote;
using boost::asio::ip::udp;

LogServer::LogServer(const fhg::log::Appender::ptr_t &appender
                   , boost::asio::io_service &io_service
                   , short port)
  : appender_(appender)
  , io_service_(io_service)
  , socket_(io_service, udp::endpoint(udp::v4(), port))
{
  LOG(INFO, "log server listening on " << udp::endpoint(udp::v4(), port));
  LOG(WARN, "FIXME: implement serialization/deserialization of LogEvents over UDP!");
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

    // TODO: deserialize LogEvent
    std::string msg(data_);

    if (msg == "QUIT deadbeef")
    {
      LOG(INFO, "got QUIT message, shutting down.");
      return;
    }

    std::clog << "got " << bytes_recv << " bytes from " << sender_endpoint_ << ": " << data_;
    appender_->append(FHGLOG_MKEVENT_HERE(INFO, data_));

    socket_.async_receive_from(
        boost::asio::buffer(data_, max_length), sender_endpoint_,
        boost::bind(&LogServer::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  else
  {
    LOG(ERROR, "error during receive: " << error);
    socket_.async_receive_from(
        boost::asio::buffer(data_, max_length), sender_endpoint_,
        boost::bind(&LogServer::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
}

