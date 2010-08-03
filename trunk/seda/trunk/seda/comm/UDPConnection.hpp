/*
   Copyright (C) 2009 Alexander Petry <alexander.petry@itwm.fraunhofer.de>.

   This file is part of seda.

   seda is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   seda is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with seda; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

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

#include <list>

#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include <seda/comm/Locator.hpp>
#include <seda/comm/Connection.hpp>

namespace seda { namespace comm {
  class UDPConnection : public Connection
  {
  public:
    UDPConnection(const Locator::ptr_t &locator
                , const std::string &logical_name);

    UDPConnection(const Locator::ptr_t &locator
                , const std::string &logical_name
                , const Location::host_t &host
                , const Location::port_t &port);

    virtual ~UDPConnection();

    const Locator::location_t::port_t &port() const { return port_; }
    const std::string &name() const { return logical_name_; }
    const Locator::location_t::host_t &host() const { return host_; }
    const Locator::ptr_t &locator() const { return locator_; }

    void start();
    void stop();

    void send(const seda::comm::SedaMessage &m);
    bool recv(seda::comm::SedaMessage &m, const bool block = true) throw(boost::thread_interrupted);

    void operator()();
  private:
    void handle_receive_from(const boost::system::error_code &error, size_t bytes_recv);
    void handle_send_to(const boost::system::error_code &error, size_t bytes_recv);

    Locator::ptr_t locator_;
    std::string logical_name_;
    Locator::location_t::host_t host_;
    Locator::location_t::port_t port_;
    boost::asio::io_service io_service_;
    boost::asio::ip::udp::endpoint sender_endpoint_;
    boost::asio::ip::udp::socket *socket_;
    std::size_t recv_waiting_;
    boost::thread *service_thread_;

    enum { max_length = (2<<16) };
    char data_[max_length];

    // asynchronous receive implementation
    boost::recursive_mutex mtx_;
    boost::condition_variable_any recv_cond_;

    typedef std::list<seda::comm::SedaMessage> message_list_t;
    message_list_t incoming_messages_;
  };
}}

#endif
