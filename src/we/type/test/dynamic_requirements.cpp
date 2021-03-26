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

#include <boost/test/unit_test.hpp>

#include <we/type/activity.hpp>
#include <we/type/net.hpp>
#include <we/type/property.hpp>

#include <util-generic/testing/random/string.hpp>

BOOST_AUTO_TEST_CASE (transition_has_no_dynamic_requirements)
{
  std::string const port (fhg::util::testing::random_identifier());
  std::string const value (fhg::util::testing::random_identifier());

  we::type::transition_t transition
    ( fhg::util::testing::random_string()
    , we::type::expression_t()
    , boost::none
    , we::type::property::type()
    , we::priority_type()
    );

  std::list<we::type::requirement_t> const static_requirements
    {  we::type::requirement_t (fhg::util::testing::random_identifier(), true)
    ,  we::type::requirement_t (fhg::util::testing::random_identifier(), false)
    ,  we::type::requirement_t (fhg::util::testing::random_identifier(), false)
    };

  for (auto const& requirement : static_requirements)
  {
    transition.add_requirement (requirement);
  }

  we::type::activity_t activity (transition);

  auto const requirements
    (activity.requirements_and_preferences (nullptr).requirements());

  BOOST_REQUIRE_EQUAL (requirements.size(), static_requirements.size());

  for ( auto lhs = requirements.begin(), rhs = static_requirements.begin()
      ; lhs != requirements.end()
      ; ++lhs, ++rhs
      )
  {
    BOOST_REQUIRE_EQUAL (lhs->value(), rhs->value());
    BOOST_REQUIRE_EQUAL (lhs->is_mandatory(), rhs->is_mandatory());
  }
}

BOOST_AUTO_TEST_CASE (transition_has_dynamic_requirements)
{
  std::string const port (fhg::util::testing::random_identifier());
  std::string const value (fhg::util::testing::random_identifier());

  we::type::property::type properties;
  properties.set ( {"fhg", "drts", "require", "dynamic_requirement"}
                 , "${" + port + "}"
                 );

  we::type::transition_t transition
    ( fhg::util::testing::random_string()
    , we::type::expression_t()
    , boost::none
    , properties
    , we::priority_type()
    );

  std::list<we::type::requirement_t> const static_requirements
    {  we::type::requirement_t (fhg::util::testing::random_identifier(), true)
    ,  we::type::requirement_t (fhg::util::testing::random_identifier(), false)
    ,  we::type::requirement_t (fhg::util::testing::random_identifier(), false)
    };

  for (auto const& requirement : static_requirements)
  {
    transition.add_requirement (requirement);
  }

  transition.add_port
    ( we::type::port_t
      ( port
      , we::type::PORT_IN
      , pnet::type::signature::signature_type (std::string ("string"))
      , we::type::property::type()
      )
    );

  we::type::activity_t activity (transition);
  activity.add_input (port, value);

  std::list<we::type::requirement_t> expected_requirements (static_requirements);
  expected_requirements.emplace_back (value, true);

  auto const requirements
    (activity.requirements_and_preferences (nullptr).requirements());

  BOOST_REQUIRE_EQUAL (requirements.size(), expected_requirements.size());

  for ( auto lhs = requirements.cbegin(), rhs = expected_requirements.cbegin()
      ; lhs != requirements.end()
      ; ++lhs, ++rhs
      )
  {
    BOOST_REQUIRE_EQUAL (lhs->value(), rhs->value());
    BOOST_REQUIRE_EQUAL (lhs->is_mandatory(), rhs->is_mandatory());
  }
}
