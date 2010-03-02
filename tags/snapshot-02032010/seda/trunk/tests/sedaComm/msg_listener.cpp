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
#include <fhglog/Configuration.hpp>
#include <seda/comm/ConnectionFactory.hpp>
#include <seda/comm/SedaMessage.hpp>

using namespace seda::comm;

int main(int argc, char **argv)
{
  fhg::log::Configurator::configure();
  if (argc < 3)
  {
    std::cerr << "usage: " << argv[0] << " name location" << std::endl;
    std::cerr << "\tname - the logical name" << std::endl;
    std::cerr << "\tthe location - ip:port pair" << std::endl;
    std::exit(1);
  }

  const std::string name(argv[1]);
  const std::string location(argv[2]);

  ConnectionFactory::ptr_t cFactory(new ConnectionFactory());
  ConnectionParameters params("udp", location, name);
  
  Connection::ptr_t conn(cFactory->createConnection(params));
  std::cerr << "I: starting connection" << std::endl;
  conn->start();
  for (;;)
  {
    SedaMessage msg;
    try {
      std::cerr << "I: listening for messages" << std::endl;
      conn->recv(msg);
      std::cout << "got message from " << conn->locator()->lookup(msg.from()) << std::endl;
      std::cout << msg.str() << std::endl;
    } catch (std::exception &ex) {
      std::cerr << "error during recv(): " << ex.what();
    }
  }
  std::exit(0);
}
