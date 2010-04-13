/*
 * =====================================================================================
 *
 *       Filename:  test_trans.cpp
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
#include <we/type/transition.hpp>
#include <we/type/place.hpp>
#include <we/type/token.hpp>

int main (int, char **)
{
  typedef place::type place_t;
  typedef token::type token_t;
  typedef unsigned int edge_t;

  typedef we::type::transition_t<place_t, edge_t, token_t> transition_t;

  transition_t t1 ("t1", transition_t::mod_type ("m", "f"));

  t1.add_ports()
    ("i", "long", we::type::PORT_IN_OUT)
    ("max", "long", we::type::PORT_IN)
  ;

  t1.add_connections()
    (transition_t::pid_t(0), "i")
    (transition_t::pid_t(1), "max")
    ("i", transition_t::pid_t(0))
  ;

  std::cout << "i (inp) = " << t1.input_port_by_name ("i") << std::endl;
  std::cout << "max (inp) = " << t1.input_port_by_name ("max") << std::endl;
  std::cout << "i (out) = " << t1.output_port_by_name ("i") << std::endl;
  std::cout << "t1.p0 = " << t1.get_port (t1.input_port_by_name ("i")) << std::endl;

  transition_t t2 ("t2", transition_t::expr_type ("true"));
  t2.add_ports()
    ("i", "long", we::type::PORT_IN)
    ("sum", "long", we::type::PORT_IN_OUT)
  ;

  std::cout << "t1=" << t1 << std::endl;
  std::cout << "t2=" << t2 << std::endl;

  {
    std::ostringstream oss;
    {
      boost::archive::text_oarchive oa (oss, boost::archive::no_header);
      oa << BOOST_SERIALIZATION_NVP (t1);
    }
    std::cout << "t1 (serialized)=" << oss.str() << std::endl;
  }
  {
    std::ostringstream oss;
    {
      boost::archive::text_oarchive oa (oss, boost::archive::no_header);
      oa << BOOST_SERIALIZATION_NVP (t2);
    }
    std::cout << "t2 (serialized)=" << oss.str() << std::endl;
  }

  return EXIT_SUCCESS;
}
