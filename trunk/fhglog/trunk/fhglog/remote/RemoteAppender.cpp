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
#include <sstream>

#include <fhglog/remote/RemoteAppender.hpp>
#include <fhglog/remote/Serialization.hpp>

using namespace fhg::log::remote;

RemoteAppender::RemoteAppender(const std::string &a_name, const std::string &a_host, short a_port)
  : Appender(a_name)
  , host_(a_host)
  , port_(a_port)
  , socket_(NULL)
{
  open();
}

RemoteAppender::~RemoteAppender()
{
  close();
}

void RemoteAppender::open()
{
  close();

  using boost::asio::ip::udp;

  udp::resolver resolver(io_service_);
  udp::resolver::query query(udp::v4(), host().c_str(), "0");
  logserver_ = *resolver.resolve(query);
  logserver_.port(port());
  socket_ = new udp::socket(io_service_);
  socket_->open(udp::v4());
}

void RemoteAppender::close()
{
  if (socket_)
  {
    socket_->close();
    delete socket_;
    socket_ = NULL;
  }
}

void RemoteAppender::append(const LogEvent &evt) const
{
  boost::system::error_code ignored_error;
  std::stringstream sstr;
  boost::archive::text_oarchive oa(sstr);
  oa << evt;
  socket_->send_to(boost::asio::buffer(sstr.str()), logserver_, 0, ignored_error);
}

