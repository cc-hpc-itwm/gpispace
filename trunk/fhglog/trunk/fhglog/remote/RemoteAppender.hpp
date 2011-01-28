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
#include <boost/thread.hpp>

#if ! defined(FHGLOG_DEFAULT_PORT)
// ascii codes of fhgl: 102 104 103 108
#define FHGLOG_DEFAULT_PORT  2438
#endif

#if ! defined(FHGLOG_DEFAULT_HOST)
#define FHGLOG_DEFAULT_HOST  "localhost"
#endif

#if ! defined(FHGLOG_DEFAULT_LOCATION)
#define FHGLOG_DEFAULT_LOCATION "localhost:2438"
#endif

namespace fhg { namespace log { namespace remote {
  class RemoteAppender : public Appender
  {
  public:
    explicit
    RemoteAppender(const std::string &name
                 , const std::string &location = FHGLOG_DEFAULT_LOCATION);
    virtual ~RemoteAppender();

    const std::string &host() const { return host_; }
    const unsigned short &port() const { return port_; }
    void append(const LogEvent &evt);
    void flush(void) {}
  private:
    void open();
    void close();

    std::string host_;
    unsigned short port_;

    boost::asio::ip::udp::socket *socket_;
    boost::asio::io_service io_service_;
    boost::asio::ip::udp::endpoint logserver_;
    std::string my_endpoint_string_;
  };
}}}

#endif
