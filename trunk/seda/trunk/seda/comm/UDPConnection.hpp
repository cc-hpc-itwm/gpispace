/*
 * =====================================================================================
 *
 *       Filename:  UDPConnection.hpp
 *
 *    Description:  implements a seda message service over udp
 *
 *        Version:  1.0
 *        Created:  10/23/2009 12:12:24 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SEDA_COMM_UDP_CONNECTION_HPP
#define SEDA_COMM_UDP_CONNECTION_HPP 1

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <seda/comm/Locator.hpp>
#include <seda/comm/Connection.hpp>

namespace seda { namespace comm {
  class UDPConnection : public Connection
  {
  public:
    UDPConnection(const Locator::ptr_t &locator
                , const std::string &logical_name
                , const std::string &host
                , short port);

    virtual ~UDPConnection();

    const short &port() const { return port_; }
    const std::string &name() const { return logical_name_; }
    const std::string &host() const { return host_; }

    void start();
    void stop();

    void send(const seda::comm::SedaMessage &m);
    bool recv(seda::comm::SedaMessage &m, const bool block = true) throw(boost::thread_interrupted);

    void handle_receive_from(const boost::system::error_code &error
                           , size_t bytes_recv);

    void operator()();
  private:
    Locator::ptr_t locator_;
    std::string logical_name_;
    std::string host_;
    short port_;
    boost::asio::io_service io_service_;
    boost::asio::ip::udp::endpoint sender_endpoint_;
    boost::asio::ip::udp::socket *socket_;
    boost::asio::ip::udp::socket *socket_out_;

    enum { max_length = ((2<<16 )-1) };
    char data_[max_length];

    // asynchronous receive implementation
    boost::recursive_mutex mtx_;
    boost::condition_variable_any recv_cond_;
    boost::thread *service_thread_;

    std::deque<seda::comm::SedaMessage> incoming_messages_;
    std::size_t recv_waiting_;
  };
}}

#endif
