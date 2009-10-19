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
#include "Serialization.hpp"

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

    DLOG(DEBUG, sender_endpoint_ << " sent me " << bytes_recv << " bytes of data: " << data_);

    if (msg == "QUIT deadbeef")
    {
      LOG(INFO, "got QUIT message, shutting down.");
      return;
    }

    try
    {
      std::stringstream sstr(msg);
      boost::archive::text_iarchive ia(sstr);

      LogEvent evt;
      ia >> evt;

      {
        // FIXME: think about a better way to introduce some kind of "trace"
        // the following is only meaningful for a flat hierarchy of logservers
        std::ostringstream ostr;
        ostr << evt.logged_via() << "@" << sender_endpoint_;
        evt.logged_via(ostr.str());
      }

      appender_->append(evt);
    }
    catch (const std::exception &ex)
    {
      LOG(WARN, "could not parse message!" << ex.what());
    }

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

