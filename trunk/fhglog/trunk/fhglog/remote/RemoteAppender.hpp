/*
 * =====================================================================================
 *
 *       Filename:  RemoteAppender.hpp
 *
 *    Description:  append to an remote logger
 *
 *        Version:  1.0
 *        Created:  10/19/2009 02:35:47 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef FHGLOG_REMOTE_APPENDER_HPP
#define FHGLOG_REMOTE_APPENDER_HPP 1

#include <boost/asio.hpp>

#include <fhglog/Appender.hpp>
#include <stdint.h>

namespace fhg { namespace log { namespace remote {
  class RemoteAppender : public Appender
  {
  public:
    RemoteAppender(const std::string &name, const std::string &host, uint16_t port);
    virtual ~RemoteAppender();

    const std::string &host() const { return host_; }
    const uint16_t &port() const { return port_; }
    void append(const LogEvent &evt) const;
  private:
    std::string host_;
    uint16_t port_;

    boost::asio::io_service io_service_;
    boost::asio::ip::udp::endpoint logserver_;
    boost::asio::ip::udp::socket *socket_;
  };
}}}

#endif
