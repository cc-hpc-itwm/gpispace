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

#include <map>
#include <string>
#include <stdexcept>

#include <seda/shared_ptr.hpp>
#include <seda/comm/Connection.hpp>

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
    ConnectionParameters(const std::string &transport
                       , const std::string &host_and_port
                       , const std::string &logical_name)
      : transport_(transport)
      , host_and_port_(host_and_port)
      , logical_name_(logical_name)
    {
    }

    ~ConnectionParameters() {}

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
      const map<string, string>::const_iterator val(props_.find(key));
      if (val == props_.end()) throw ParameterNotFound(key);
      return val->second;
    }

    const std::string &transport() const { return transport_; }
    const std::string &host() const { return host_and_port_; }
    const std::string &name() const { return logical_name_; }
  private:
    std::string transport_;
    std::string host_and_port_;
    std::string logical_name_;
    std::map<std::string, std::string> props_;
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
  protected:
    virtual ConnectionParameters parse_uri(const std::string &uri);
  };
}}

#endif
