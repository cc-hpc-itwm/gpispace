/*
 * =====================================================================================
 *
 *       Filename:  msg_listener.cpp
 *
 *    Description:  A simple listener process that simply waits for messages
 *                  and prints them on stdout
 *
 *        Version:  1.0
 *        Created:  09/11/2009 01:46:06 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <iostream> // std::cout
#include <cstdlib>
#include <seda/comm/ConnectionFactory.hpp>
#include <seda/comm/SedaMessage.hpp>

using namespace seda::comm;

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cerr << "usage: " << argv[0] << " name" << std::endl;
    std::cerr << "\tname - the logical to listen on" << std::endl;
    std::exit(1);
  }
  ConnectionFactory::ptr_t cFactory(new ConnectionFactory());
  ConnectionParameters params("zmq", "localhost", argv[1]);
  params.put("iface_in",  "*");
  params.put("iface_out", "*");
  
  Connection::ptr_t conn(cFactory->createConnection(params));
  std::cerr << "I: starting connection" << std::endl;
  conn->start();
  for (;;)
  {
    SedaMessage msg;
    try {
      std::cerr << "I: listening for messages" << std::endl;
      conn->recv(msg);
      std::cout << "received message: " << msg.str() << std::endl;
    } catch (std::exception &ex) {
      std::cerr << "error during recv(): " << ex.what();
    }
  }
  std::exit(0);
}
