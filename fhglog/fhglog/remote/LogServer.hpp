/*
 * =====================================================================================
 *
 *       Filename:  LogServer.hpp
 *
 *    Description:  implements a logging server
 *
 *        Version:  1.0
 *        Created:  10/19/2009 08:13:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef FHGLOG_REMOTE_LOG_SERVER_HPP
#define FHGLOG_REMOTE_LOG_SERVER_HPP 1

#include <boost/asio.hpp>

#include <fhglog/Appender.hpp> // to write received events to custom appender
#include <fhglog/remote/RemoteAppender.hpp> // to forward events to another logserver

namespace fhg { namespace log { namespace remote {
  class LogServer
  {
  public:
    explicit
    LogServer(const fhg::log::Appender::ptr_t &appender
            , boost::asio::io_service &io_service
            , unsigned short port);
    ~LogServer() {}

    void handle_receive_from(const boost::system::error_code &error
                           , size_t bytes_recv);
  private:
    fhg::log::Appender::ptr_t appender_;
    //    boost::asio::io_service &io_service_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint sender_endpoint_;

    enum { max_length = (2<<16) };
    char data_[max_length];
  };
}}}

#endif
