/*
 * =====================================================================================
 *
 *       Filename:  ConnectionFactory.cpp
 *
 *    Description:  implementation of the connectionfactory
 *
 *        Version:  1.0
 *        Created:  09/08/2009 04:15:54 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include "ConnectionFactory.hpp"
#include "ZMQConnection.hpp"

using namespace seda::comm;

ConnectionParameters ConnectionFactory::parse_uri(const std::string &uri)
{
  ConnectionParameters params("tcp", "localhost", "foobar");
  params.put("protocol", "zmq");
  params.put("uri", uri);
  return params;
}

Connection::ptr_t ConnectionFactory::createConnection(const ConnectionParameters &params)
{
  if (params.transport() == "zmq")
  {
    return Connection::ptr_t(
      new ZMQConnection(
          params.host()
        , params.name()
        , params.get("iface_in")
        , params.get("iface_out")
      )
    );
  }
  throw std::runtime_error(std::string("no suitable connection found for protocol: ") + params.get("proto"));
}
