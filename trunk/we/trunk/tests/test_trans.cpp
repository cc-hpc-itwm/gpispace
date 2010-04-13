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
  transition_t t2 ("t2", transition_t::expr_type ("true"));

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

  return EXIT_SUCCESS;
}
