// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <we/type/Activity.hpp>
//! \todo: eliminate this include that just completes the type
#include <we/type/net.hpp>
#include <we/type/property.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/random/string.hpp>

#include <boost/optional/optional_io.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

namespace
{
  using F = std::function<::boost::optional<unsigned long> (we::type::Activity)>;
  std::vector<F> expected_property_values
  { [] (we::type::Activity activity)
    {
      return activity.requirements_and_preferences (nullptr).numWorkers();
    }
  , [] (we::type::Activity activity)
    {
      return activity.requirements_and_preferences (nullptr).maximum_number_of_retries();
    }
  };
}

BOOST_AUTO_TEST_CASE (get_schedule_data_not_set)
{
  we::type::Transition const transition
    ( fhg::util::testing::random_string()
    , we::type::Expression()
    , ::boost::none
    , we::type::property::type()
    , we::priority_type()
    , ::boost::optional<we::type::eureka_id_type>{}
    , std::list<we::type::Preference>{}
    );
  we::type::Activity activity (transition);

  BOOST_REQUIRE (activity.requirements_and_preferences (nullptr).numWorkers());
  BOOST_REQUIRE
    (!activity.requirements_and_preferences (nullptr).maximum_number_of_retries());
}

BOOST_DATA_TEST_CASE
  ( get_schedule_data_constant_string
  , ::boost::unit_test::data::make ({"num_worker", "maximum_number_of_retries"})
  ^ ::boost::unit_test::data::xrange (2)
  , property_name
  , k
  )
{
  unsigned long const value {fhg::util::testing::random<unsigned long>()()};

  we::type::property::type properties;
  properties.set ( {"fhg", "drts", "schedule", property_name}
                 , std::to_string (value) + "UL"
                 );

  we::type::Transition const transition
    ( fhg::util::testing::random_string()
    , we::type::Expression()
    , ::boost::none
    , properties
    , we::priority_type()
    , ::boost::optional<we::type::eureka_id_type>{}
    , std::list<we::type::Preference>{}
    );
  we::type::Activity activity (transition);

  BOOST_REQUIRE_EQUAL (expected_property_values[k] (activity), value);
}

BOOST_DATA_TEST_CASE
  ( get_schedule_data_expression_simple
  , ::boost::unit_test::data::make ({"num_worker", "maximum_number_of_retries"})
  ^ ::boost::unit_test::data::xrange (2)
  , property_name
  , k
  )
{
  std::string const port_name (fhg::util::testing::random_identifier());
  auto const value (fhg::util::testing::random<unsigned long>{}());

  we::type::property::type properties;
  properties.set ( {"fhg", "drts", "schedule", property_name}
                 , "${" + port_name + "}"
                 );

  we::type::Transition transition
    ( fhg::util::testing::random_string()
    , we::type::Expression()
    , ::boost::none
    , properties
    , we::priority_type()
    , ::boost::optional<we::type::eureka_id_type>{}
    , std::list<we::type::Preference>{}
    );

  transition.add_port
    ( we::type::Port
      ( port_name
      , we::type::port::direction::In{}
      , pnet::type::signature::signature_type (std::string ("unsigned long"))
      , we::type::property::type()
      )
    );

  we::type::Activity activity (transition);
  activity.add_input (port_name, value);

  BOOST_REQUIRE_EQUAL (expected_property_values[k] (activity), value);
}

struct random_identifier
{
  std::string operator()() const
  {
    return fhg::util::testing::random_identifier();
  }
};
using unique_random_identifier
  = fhg::util::testing::unique_random<std::string, random_identifier>;

BOOST_AUTO_TEST_CASE (get_schedule_data_expression_sum)
{
  unique_random_identifier port_names;
  std::string const port_name1 (port_names());
  std::string const port_name2 (port_names());
  unsigned long const value1 {fhg::util::testing::random<unsigned long>()()};
  unsigned long const value2 {fhg::util::testing::random<unsigned long>()()};

  we::type::property::type properties;
  properties.set ( {"fhg", "drts", "schedule", "num_worker"}
                 , "${" + port_name1 + "} + ${" + port_name2 + "}"
                 );

  we::type::Transition transition
    ( fhg::util::testing::random_string()
    , we::type::Expression()
    , ::boost::none
    , properties
    , we::priority_type()
    , ::boost::optional<we::type::eureka_id_type>{}
    , std::list<we::type::Preference>{}
    );

  transition.add_port
    ( we::type::Port
      ( port_name1
      , we::type::port::direction::In{}
      , pnet::type::signature::signature_type (std::string ("unsigned long"))
      , we::type::property::type()
      )
    );
  transition.add_port
    ( we::type::Port
      ( port_name2
      , we::type::port::direction::In{}
      , pnet::type::signature::signature_type (std::string ("unsigned long"))
        , we::type::property::type()
      )
    );

  we::type::Activity activity (transition);
  activity.add_input (port_name1, value1);
  activity.add_input (port_name2, value2);

  BOOST_REQUIRE_EQUAL
    (activity.requirements_and_preferences (nullptr).numWorkers(), value1 + value2);
}
