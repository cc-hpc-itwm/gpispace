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

#include <boost/array.hpp>

#include <fhglog/remote/RemoteAppender.hpp>
#include <fhglog/remote/Serialization.hpp>

using namespace fhg::log::remote;

RemoteAppender::RemoteAppender(const std::string &a_name, const std::string &a_host, const std::string &a_service)
  : Appender(a_name)
  , host_(a_host)
  , service_(a_service)
{
  using boost::asio::ip::udp;

  udp::resolver resolver(io_service_);
  udp::resolver::query query(udp::v4(), a_host.c_str(), a_service);
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

void RemoteAppender::append(const LogEvent &evt) const
{
  boost::system::error_code ignored_error;
  std::string message(getFormat()->format(evt)); // FIXME: put serialized logeven here
  socket_->send_to(boost::asio::buffer(message), logserver_, 0, ignored_error);
}

