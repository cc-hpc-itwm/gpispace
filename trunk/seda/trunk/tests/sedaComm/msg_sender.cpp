/*
 * =====================================================================================
 *
 *       Filename:  msg_listener.cpp
 *
 *    Description:  A simple sender process that simply sends a message to a given destination
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
#include <fstream>
#include <sstream>
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
    std::cerr << "usage: " << argv[0] << " from to [payload]" << std::endl;
    std::cerr << "     from - the logical name to send messages from" << std::endl;
    std::cerr << "       to - the logical name to send messages to" << std::endl;
    std::cerr << "  payload - the message body (if not specified or equals to -, stdin is taken)" << std::endl;
    std::exit(1);
  }
  const std::string from(argv[1]);
  const std::string to(argv[2]);
  std::string payload_src("-");
  if (argc > 3) {
    payload_src = argv[3];
  }

  ConnectionFactory::ptr_t cFactory(new ConnectionFactory());
  ConnectionParameters params("udp", "127.0.0.1:0", from);
  
  Connection::ptr_t conn(cFactory->createConnection(params));
  conn->locator()->insert(to, "127.0.0.1:5222");
  std::cerr << "I: starting connection" << std::endl;
  conn->start();

  // build the payload
  std::string payload;
  if (payload_src == "-") {
    std::cerr << "D: reading payload from stdin" << std::endl;
    std::stringstream istr;
    std::cin >> istr.rdbuf();
    payload = istr.str();
  } else {
    std::ifstream ifs(payload_src.c_str(), std::ios::in | std::ios::binary);
    if (! ifs.good()) {
      std::cerr << "E: could not read from payload-file " << payload_src << std::endl;
      return 1;
    } else {
      std::cerr << "D: reading payload from " << payload_src << std::endl;
      std::stringstream istr;
      ifs >> istr.rdbuf();
      payload = istr.str();
    }
  }
  std::cerr << "D: payload = " << payload << std::endl;

  SedaMessage msg(from, to, payload, 0);
  try {
    std::cout << "sending message: " << msg.str() << std::endl;
    conn->send(msg);
  } catch (std::exception &ex) {
    std::cerr << "error during send(): " << ex.what();
  }
  std::exit(0);
}
