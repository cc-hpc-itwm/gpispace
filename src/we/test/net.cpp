#define BOOST_TEST_MODULE net
#include <boost/test/unit_test.hpp>

#include <we/type/expression.hpp>
#include <we/type/net.hpp>
#include <we/type/transition.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/random_string.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/random/random_device.hpp>

#include <sstream>

BOOST_AUTO_TEST_CASE (transition_without_input_port_can_not_fire)
{
  we::type::net_type net;
  net.add_transition ( we::type::transition_t
                       ( fhg::util::random_string()
                       , we::type::expression_t()
                       , boost::none
                       , true
                       , we::type::property::type()
                       , we::priority_type()
                       )
                     );

  boost::random::random_device random_engine;

  BOOST_REQUIRE
    (!net.fire_expressions_and_extract_activity_random (random_engine));
}

BOOST_AUTO_TEST_CASE (deserialized_transition_without_input_port_can_not_fire)
{
  std::stringstream iostream;
  boost::archive::text_oarchive oar (iostream);

  {
    we::type::net_type net;
    net.add_transition ( we::type::transition_t
                         ( fhg::util::random_string()
                         , we::type::expression_t()
                         , boost::none
                         , true
                         , we::type::property::type()
                         , we::priority_type()
                         )
                       );

    oar << net;
  }

  boost::archive::text_iarchive iar (iostream);

  we::type::net_type net;
  iar >> net;

  boost::random::random_device random_engine;

  BOOST_REQUIRE
    (!net.fire_expressions_and_extract_activity_random (random_engine));
}
