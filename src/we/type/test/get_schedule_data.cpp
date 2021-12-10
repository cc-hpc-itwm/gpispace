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

#include <we/type/Activity.hpp>
//! \todo: eliminate this include that just completes the type
#include <we/type/net.hpp>
#include <we/type/property.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random/string.hpp>
#include <util-generic/testing/random.hpp>

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
}

BOOST_AUTO_TEST_CASE (get_schedule_data_constant_string)
{
  unsigned long const value {fhg::util::testing::random<unsigned long>()()};

  we::type::property::type properties;
  properties.set ( {"fhg", "drts", "schedule", "num_worker"}
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

  BOOST_REQUIRE_EQUAL (activity.requirements_and_preferences (nullptr).numWorkers(), value);
}

BOOST_AUTO_TEST_CASE (get_schedule_data_expression_simple)
{
  std::string const port_name (fhg::util::testing::random_identifier());
  auto const value (fhg::util::testing::random<unsigned long>{}());

  we::type::property::type properties;
  properties.set ( {"fhg", "drts", "schedule", "num_worker"}
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

  BOOST_REQUIRE_EQUAL (activity.requirements_and_preferences (nullptr).numWorkers(), value);
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
