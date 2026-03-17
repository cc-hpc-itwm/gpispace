// Copyright (C) 2019-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <test/we/operator_equal.hpp>
#include <gspc/we/type/Activity.hpp>
#include <gspc/we/type/net.hpp>
#include <gspc/we/type/property.hpp>

#include <gspc/testing/printer/generic.hpp>
#include <gspc/testing/printer/list.hpp>
#include <gspc/testing/random.hpp>

#include <optional>
#include <boost/test/unit_test.hpp>

#include <list>
#include <string>

GSPC_BOOST_TEST_LOG_VALUE_PRINTER (gspc::we::type::Requirement, os, x)
{
  os << x.value();
}

BOOST_AUTO_TEST_CASE (transition_has_no_dynamic_requirements)
{
  gspc::we::type::Transition transition
    ( gspc::testing::random_string()
    , gspc::we::type::Expression()
    , std::nullopt
    , gspc::we::type::property::type()
    , gspc::we::priority_type()
    , std::optional<gspc::we::type::eureka_id_type>{}
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  gspc::testing::unique_random<std::string> requirement_names;
  std::list<gspc::we::type::Requirement> const static_requirements
    { gspc::we::type::Requirement (requirement_names())
    , gspc::we::type::Requirement (requirement_names())
    , gspc::we::type::Requirement (requirement_names())
    };

  for (auto const& requirement : static_requirements)
  {
    transition.add_requirement (requirement);
  }

  gspc::we::type::Activity activity (transition);

  auto const requirements
    (activity.requirements_and_preferences (nullptr).requirements());

  BOOST_REQUIRE_EQUAL (requirements, static_requirements);
}

BOOST_AUTO_TEST_CASE (transition_has_dynamic_requirements)
{
  std::string const port (gspc::testing::random_identifier());

  gspc::we::type::property::type properties;
  properties.set ( {"fhg", "drts", "require", "dynamic_requirement"}
                 , "${" + port + "}"
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

  gspc::testing::unique_random<std::string> requirement_names;
  std::list<gspc::we::type::Requirement> const static_requirements
    {  gspc::we::type::Requirement (requirement_names())
    ,  gspc::we::type::Requirement (requirement_names())
    ,  gspc::we::type::Requirement (requirement_names())
    };

  for (auto const& requirement : static_requirements)
  {
    transition.add_requirement (requirement);
  }

  transition.add_port
    ( gspc::we::type::Port
      ( port
      , gspc::we::type::port::direction::In{}
      , gspc::pnet::type::signature::signature_type (std::string ("string"))
      , gspc::we::type::property::type()
      )
    );

  gspc::we::type::Activity activity (transition);

  auto const value (requirement_names());
  activity.add_input (port, value);

  auto expected_requirements (static_requirements);
  expected_requirements.emplace_back (value);

  auto const requirements
    (activity.requirements_and_preferences (nullptr).requirements());

  BOOST_REQUIRE_EQUAL (requirements, expected_requirements);
}
