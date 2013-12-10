/*
 * =====================================================================================
 *
 *       Filename:  test_port.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  04/12/2010 12:18:50 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <we/type/signature.hpp>
#include <we/type/port.hpp>
#include <we/serialize/unordered_map.hpp>

int main (int, char **)
{
  we::type::port_t p1("Max", we::type::PORT_IN, pnet::type::signature::signature_type());
  we::type::port_t p2;

  {
    std::ostringstream oss;
    {
      boost::archive::text_oarchive oa (oss, boost::archive::no_header);
      oa << BOOST_SERIALIZATION_NVP (p1);
    }

    std::cout << "Port 1: " << p1 << std::endl;
    std::cout << "Port 1 (serialized): " << oss.str() << std::endl;

    if (p2.name() != "default")
    {
      std::cerr << "E: default name of port changed" << std::endl;
      return EXIT_FAILURE;
    }

    std::cout << "Port 2 (original): " << p2 << std::endl;
    {
      std::istringstream iss (oss.str());
      boost::archive::text_iarchive ia (iss, boost::archive::no_header);
      ia >> BOOST_SERIALIZATION_NVP (p2);
    }
    std::cout << "Port 2 (deserialized): " << p2 << std::endl;
  }

  we::type::port_t p3;
  std::cout << "Port 3: " << p3 << std::endl;
  {
    std::ostringstream oss;
    {
      boost::archive::text_oarchive oa (oss, boost::archive::no_header);
      oa << BOOST_SERIALIZATION_NVP (p3);
    }
    std::cout << "Port 3 (serialized): " << p3 << std::endl;
  }

  return EXIT_SUCCESS;
}
