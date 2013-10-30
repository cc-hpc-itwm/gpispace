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
 *       Filename:  ConnectionFactory.hpp
 *
 *    Description:  Creates different kinds of connections
 *
 *        Version:  1.0
 *        Created:  09/08/2009 03:30:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SEDA_COMM_CONNECTION_FACTORY_HPP
#define SEDA_COMM_CONNECTION_FACTORY_HPP 1

#include <string>
#include <stdexcept>

#include <boost/unordered_map.hpp>
#include <seda/shared_ptr.hpp>
#include <seda/comm/Connection.hpp>

#include <sstream>

namespace seda { namespace comm {
  /**
   * Holds information about connection parameters.
   *
   * Examples:
   *    - transport protocol (tcp/udp/etc.)
   *    - protocol (zmq,stomp,amqp)
   *    - host/port
   *    - own logical name
   *    - username/password used to connect to host/port (if required)
   *
   * uri: zmq://host:port/logicalname?username=guest&password=guest
   */
  class ConnectionParameters {
  public:
    typedef boost::unordered_map<std::string, std::string> prop_map_t;

//    ConnectionParameters(const std::string &a_transport
//                       , const std::string &host
//                       , const std::string &a_logical_name
//                       , unsigned short port
//                       )
//      : transport_(a_transport)
//      , host_(host)
//      , logical_name_(a_logical_name)
//      , port_(port)
//    {
//    }

    ConnectionParameters(const std::string &a_transport
                       , const std::string &host_and_port
                       , const std::string &a_logical_name
                       )
      : transport_(a_transport)
      , logical_name_(a_logical_name)
    {
      std::string::size_type colon_pos = host_and_port.find_first_of(':');
      if (colon_pos == std::string::npos)
      {
        host_ = host_and_port;
        port_ = 0;
      }
      else
      {
        host_ = host_and_port.substr(0, colon_pos);
        std::stringstream sstr(host_and_port.substr(colon_pos+1));
        sstr >> port_;
        if (! sstr)
        {
          throw std::runtime_error("illegal argument: " + host_and_port);
        }
      }
    }

    class ParameterNotFound : public std::runtime_error
    {
    public:
      ParameterNotFound(const std::string &key)
        : std::runtime_error(std::string("key not found in connection parameters: ") + key)
        {}
    };

    void put(const std::string &key, const std::string &val)
    {
      props_.insert(std::make_pair(key,val));
    }

    const std::string &get(const std::string &key) const throw (ParameterNotFound)
    {
      using namespace std;
      const prop_map_t::const_iterator val(props_.find(key));
      if (val == props_.end()) throw ParameterNotFound(key);
      return val->second;
    }

    const std::string &transport() const { return transport_; }
    const std::string &host() const { return host_; }
    const std::string &name() const { return logical_name_; }
    const unsigned short &port() const { return port_; }
  private:
    std::string transport_;
    std::string host_;
    std::string logical_name_;
    unsigned short port_;
    prop_map_t props_;
  };

  class ConnectionFactory {
  public:
    typedef seda::shared_ptr<ConnectionFactory> ptr_t;

    /**
     * Creates a new Connection defined by the given uri
     *
     */
    virtual Connection::ptr_t createConnection(const std::string &uri)
    {
      return createConnection(parse_uri(uri));
    }
    virtual Connection::ptr_t createConnection(const ConnectionParameters &params);
    virtual Connection::ptr_t createConnection(const ConnectionParameters &params, const Locator::ptr_t &locator);
    virtual ~ConnectionFactory() {}
  protected:
    virtual ConnectionParameters parse_uri(const std::string &uri);
  };
}}

#endif
