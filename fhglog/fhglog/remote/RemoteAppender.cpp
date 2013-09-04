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

#include <fhglog/util.hpp>
#include <fhglog/fhglog.hpp>
#include <fhglog/remote/RemoteAppender.hpp>
#include <boost/lexical_cast.hpp>

using namespace fhg::log::remote;

RemoteAppender::RemoteAppender(const std::string &a_name, const std::string &location)
  : Appender(a_name)
  , socket_(NULL)
{
  std::pair<std::string, std::string> host_port = fhg::log::split_string(location, ":");
  host_ = host_port.first;
  if (host_port.second.empty())
  {
    host_port.second = boost::lexical_cast<std::string>(FHGLOG_DEFAULT_PORT);
  }

  std::stringstream sstr(host_port.second);
  sstr >> port_;
  if (!sstr)
  {
    throw std::runtime_error("could not parse port information: " + host_port.second);
  }

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
  socket_ = new udp::socket(io_service_, udp::v4());

  my_endpoint_string_ = boost::asio::ip::host_name();
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

void RemoteAppender::append(const LogEvent &evt)
{
  boost::system::error_code ignored_error;
  std::stringstream sstr;
  evt.encode (sstr);
  socket_->send_to(boost::asio::buffer(sstr.str ()), logserver_, 0, ignored_error);
}
