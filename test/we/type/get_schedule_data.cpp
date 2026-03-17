// Copyright (C) 2014-2016,2018,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/we/type/Activity.hpp>
//! \todo: eliminate this include that just completes the type
#include <gspc/we/type/net.hpp>
#include <gspc/we/type/property.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/random/string.hpp>

#include <gspc/testing/printer/optional.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

namespace
{
  std::optional<unsigned long> get_num_workers (gspc::we::type::Activity activity)
  {
    return activity.requirements_and_preferences (nullptr).numWorkers();
  }

  std::optional<unsigned long> get_max_retries (gspc::we::type::Activity activity)
  {
    return activity.requirements_and_preferences (nullptr).maximum_number_of_retries();
  }

  using F = std::function<std::optional<unsigned long> (gspc::we::type::Activity)>;

  std::vector<F> expected_property_values {&get_num_workers, &get_max_retries};
}

BOOST_AUTO_TEST_CASE (get_schedule_data_not_set)
{
  gspc::we::type::Transition const transition
    ( gspc::testing::random_string()
    , gspc::we::type::Expression()
    , std::nullopt
    , gspc::we::type::property::type()
    , gspc::we::priority_type()
    , std::optional<gspc::we::type::eureka_id_type>{}
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );
  gspc::we::type::Activity activity (transition);

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
  unsigned long const value {gspc::testing::random<unsigned long>()()};

  gspc::we::type::property::type properties;
  properties.set ( {"fhg", "drts", "schedule", property_name}
                 , std::to_string (value) + "UL"
                 );

  gspc::we::type::Transition const transition
    ( gspc::testing::random_string()
    , gspc::we::type::Expression()
    , std::nullopt
    , properties
    , gspc::we::priority_type()
    , std::optional<gspc::we::type::eureka_id_type>{}
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );
  gspc::we::type::Activity activity (transition);

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
  std::string const port_name (gspc::testing::random_identifier());
  auto const value (gspc::testing::random<unsigned long>{}());

  gspc::we::type::property::type properties;
  properties.set ( {"fhg", "drts", "schedule", property_name}
                 , "${" + port_name + "}"
                 );

  gspc::we::type::Transition transition
    ( gspc::testing::random_string()
    , gspc::we::type::Expression()
    , std::nullopt
    , properties
    , gspc::we::priority_type()
    , std::optional<gspc::we::type::eureka_id_type>{}
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  transition.add_port
    ( gspc::we::type::Port
      ( port_name
      , gspc::we::type::port::direction::In{}
      , gspc::pnet::type::signature::signature_type (std::string ("unsigned long"))
      , gspc::we::type::property::type()
      )
    );

  gspc::we::type::Activity activity (transition);
  activity.add_input (port_name, value);

  BOOST_REQUIRE_EQUAL (expected_property_values[k] (activity), value);
}

struct random_identifier
{
  std::string operator()() const
  {
    return gspc::testing::random_identifier();
  }
};
using unique_random_identifier
  = gspc::testing::unique_random<std::string, random_identifier>;

BOOST_AUTO_TEST_CASE (get_schedule_data_expression_sum)
{
  unique_random_identifier port_names;
  std::string const port_name1 (port_names());
  std::string const port_name2 (port_names());
  unsigned long const value1 {gspc::testing::random<unsigned long>()()};
  unsigned long const value2 {gspc::testing::random<unsigned long>()()};

  gspc::we::type::property::type properties;
  properties.set ( {"fhg", "drts", "schedule", "num_worker"}
                 , "${" + port_name1 + "} + ${" + port_name2 + "}"
                 );

  gspc::we::type::Transition transition
    ( gspc::testing::random_string()
    , gspc::we::type::Expression()
    , std::nullopt
    , properties
    , gspc::we::priority_type()
    , std::optional<gspc::we::type::eureka_id_type>{}
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  transition.add_port
    ( gspc::we::type::Port
      ( port_name1
      , gspc::we::type::port::direction::In{}
      , gspc::pnet::type::signature::signature_type (std::string ("unsigned long"))
      , gspc::we::type::property::type()
      )
    );
  transition.add_port
    ( gspc::we::type::Port
      ( port_name2
      , gspc::we::type::port::direction::In{}
      , gspc::pnet::type::signature::signature_type (std::string ("unsigned long"))
        , gspc::we::type::property::type()
      )
    );

  gspc::we::type::Activity activity (transition);
  activity.add_input (port_name1, value1);
  activity.add_input (port_name2, value2);

  BOOST_REQUIRE_EQUAL
    (activity.requirements_and_preferences (nullptr).numWorkers(), value1 + value2);
}
