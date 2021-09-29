// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <we/test/operator_equal.hpp>
#include <we/type/Activity.hpp>
#include <we/type/net.hpp>
#include <we/type/property.hpp>

#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/printer/list.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/optional.hpp>
#include <boost/test/unit_test.hpp>

#include <list>
#include <string>

FHG_BOOST_TEST_LOG_VALUE_PRINTER (we::type::Requirement, os, x)
{
  os << x.value();
}

BOOST_AUTO_TEST_CASE (transition_has_no_dynamic_requirements)
{
  we::type::Transition transition
    ( fhg::util::testing::random_string()
    , we::type::Expression()
    , boost::none
    , we::type::property::type()
    , we::priority_type()
    , boost::optional<we::type::eureka_id_type>{}
    , std::list<we::type::Preference>{}
    );

  fhg::util::testing::unique_random<std::string> requirement_names;
  std::list<we::type::Requirement> const static_requirements
    { we::type::Requirement (requirement_names())
    , we::type::Requirement (requirement_names())
    , we::type::Requirement (requirement_names())
    };

  for (auto const& requirement : static_requirements)
  {
    transition.add_requirement (requirement);
  }

  we::type::Activity activity (transition);

  auto const requirements
    (activity.requirements_and_preferences (nullptr).requirements());

  BOOST_REQUIRE_EQUAL (requirements, static_requirements);
}

BOOST_AUTO_TEST_CASE (transition_has_dynamic_requirements)
{
  std::string const port (fhg::util::testing::random_identifier());

  we::type::property::type properties;
  properties.set ( {"fhg", "drts", "require", "dynamic_requirement"}
                 , "${" + port + "}"
                 );

  we::type::Transition transition
    ( fhg::util::testing::random_string()
    , we::type::Expression()
    , boost::none
    , properties
    , we::priority_type()
    , boost::optional<we::type::eureka_id_type>{}
    , std::list<we::type::Preference>{}
    );

  fhg::util::testing::unique_random<std::string> requirement_names;
  std::list<we::type::Requirement> const static_requirements
    {  we::type::Requirement (requirement_names())
    ,  we::type::Requirement (requirement_names())
    ,  we::type::Requirement (requirement_names())
    };

  for (auto const& requirement : static_requirements)
  {
    transition.add_requirement (requirement);
  }

  transition.add_port
    ( we::type::Port
      ( port
      , we::type::port::direction::In{}
      , pnet::type::signature::signature_type (std::string ("string"))
      , we::type::property::type()
      )
    );

  we::type::Activity activity (transition);

  auto const value (requirement_names());
  activity.add_input (port, value);

  auto expected_requirements (static_requirements);
  expected_requirements.emplace_back (value);

  auto const requirements
    (activity.requirements_and_preferences (nullptr).requirements());

  BOOST_REQUIRE_EQUAL (requirements, expected_requirements);
}
