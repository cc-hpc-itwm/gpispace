/*
 * =====================================================================================
 *
 *       Filename:  RemoteAppender.cpp
 *
 *    Description:  implementation of the remote appender
 *
 *        Version:  1.0
 *        Created:  10/19/2009 02:38:48 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <fhglog/remote/RemoteAppender.hpp>
#include <boost/array.hpp>

using namespace fhg::log::remote;

RemoteAppender::RemoteAppender(const std::string &a_name, const std::string &a_host, uint16_t a_port)
  : Appender(a_name)
  , host_(a_host)
  , port_(a_port)
{
  using boost::asio::ip::udp;

  udp::resolver resolver(io_service_);
  udp::resolver::query query(udp::v4(), a_host, a_port);
  logserver_ = *resolver.resolve(query);
  socket_ = new udp::socket(io_service_);
  socket_->open(udp::v4());
}

RemoteAppender::~RemoteAppender()
{
  if (socket_)
    delete socket_;
  socket_ = NULL;
}

void RemoteAppender::append(const LogEvent &/*evt*/) const
{
  // serialize event and send it
  boost::array<char, 1> send_buf;
  socket_->send_to(boost::asio::buffer(send_buf), logserver_);
}
