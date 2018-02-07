#include <boost/test/unit_test.hpp>

#include <we/type/activity.hpp>
//! \todo: eliminate this include that just completes the type
#include <we/type/net.hpp>
#include <we/type/property.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random/string.hpp>
#include <util-generic/testing/random.hpp>

BOOST_AUTO_TEST_CASE (get_schedule_data_not_set)
{
  we::type::transition_t const transition
    ( fhg::util::testing::random_string()
    , we::type::expression_t()
    , boost::none
    , we::type::property::type()
    , we::priority_type()
    );
  we::type::activity_t const activity (transition, boost::none);

  BOOST_REQUIRE (!activity.get_schedule_data().num_worker());
}

BOOST_AUTO_TEST_CASE (get_schedule_data_constant_string)
{
  unsigned long const value {fhg::util::testing::random<unsigned long>()()};

  we::type::property::type properties;
  properties.set ( {"fhg", "drts", "schedule", "num_worker"}
                 , std::to_string (value) + "UL"
                 );

  we::type::transition_t const transition
    ( fhg::util::testing::random_string()
    , we::type::expression_t()
    , boost::none
    , properties
    , we::priority_type()
    );
  we::type::activity_t const activity (transition, boost::none);

  BOOST_REQUIRE (!!activity.get_schedule_data().num_worker());
  BOOST_REQUIRE_EQUAL (activity.get_schedule_data().num_worker().get(), value);
}

BOOST_AUTO_TEST_CASE (get_schedule_data_expression_simple)
{
  std::string const port_name (fhg::util::testing::random_identifier());
  unsigned long const value (rand());

  we::type::property::type properties;
  properties.set ( {"fhg", "drts", "schedule", "num_worker"}
                 , "${" + port_name + "}"
                 );

  we::type::transition_t transition
    ( fhg::util::testing::random_string()
    , we::type::expression_t()
    , boost::none
    , properties
    , we::priority_type()
    );

  we::port_id_type const port_id
    ( transition.add_port
      ( we::type::port_t
        ( port_name
        , we::type::PORT_IN
        , pnet::type::signature::signature_type (std::string ("unsigned long"))
        , we::type::property::type()
        )
      )
    );

  we::type::activity_t activity (transition, boost::none);
  activity.add_input (port_id, value);

  BOOST_REQUIRE (!!activity.get_schedule_data().num_worker());
  BOOST_REQUIRE_EQUAL (activity.get_schedule_data().num_worker().get(), value);
}

BOOST_AUTO_TEST_CASE (get_schedule_data_expression_sum)
{
  std::string const port_name1 (fhg::util::testing::random_identifier());
  std::string const port_name2 (fhg::util::testing::random_identifier());
  unsigned long const value1 {fhg::util::testing::random<unsigned long>()()};
  unsigned long const value2 {fhg::util::testing::random<unsigned long>()()};

  we::type::property::type properties;
  properties.set ( {"fhg", "drts", "schedule", "num_worker"}
                 , "${" + port_name1 + "} + ${" + port_name2 + "}"
                 );

  we::type::transition_t transition
    ( fhg::util::testing::random_string()
    , we::type::expression_t()
    , boost::none
    , properties
    , we::priority_type()
    );

  we::port_id_type const port_id1
    ( transition.add_port
      ( we::type::port_t
        ( port_name1
        , we::type::PORT_IN
        , pnet::type::signature::signature_type (std::string ("unsigned long"))
        , we::type::property::type()
        )
      )
    );
  we::port_id_type const port_id2
    ( transition.add_port
      ( we::type::port_t
        ( port_name2
        , we::type::PORT_IN
        , pnet::type::signature::signature_type (std::string ("unsigned long"))
        , we::type::property::type()
        )
      )
    );

  we::type::activity_t activity (transition, boost::none);
  activity.add_input (port_id1, value1);
  activity.add_input (port_id2, value2);

  BOOST_REQUIRE (!!activity.get_schedule_data().num_worker());
  BOOST_REQUIRE_EQUAL
    (activity.get_schedule_data().num_worker().get(), value1 + value2);
}
