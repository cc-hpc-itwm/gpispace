// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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
#include <we/type/Expression.hpp>
#include <we/type/MemoryBufferInfo.hpp>
#include <we/type/ModuleCall.hpp>
#include <we/type/net.hpp>
#include <we/type/Transition.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/random/string.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

#include <sstream>

#include <we/test/net.common.hpp>

BOOST_AUTO_TEST_CASE (transition_without_input_port_can_not_fire)
{
  we::type::net_type net;
  net.add_transition ( we::type::Transition
                       ( fhg::util::testing::random_string()
                       , we::type::Expression()
                       , ::boost::none
                       , no_properties()
                       , we::priority_type()
                       , ::boost::optional<we::type::eureka_id_type>{}
                       , std::list<we::type::Preference>{}
                       )
                     );

  BOOST_REQUIRE
    ( !net.fire_expressions_and_extract_activity_random_TESTING_ONLY
        (random_engine(), unexpected_workflow_response, unexpected_eureka)
    );
}

BOOST_AUTO_TEST_CASE (deserialized_transition_without_input_port_can_not_fire)
{
  std::stringstream iostream;
  ::boost::archive::text_oarchive oar (iostream);

  {
    we::type::net_type net;
    net.add_transition ( we::type::Transition
                         ( fhg::util::testing::random_string()
                         , we::type::Expression()
                         , ::boost::none
                         , no_properties()
                         , we::priority_type()
                         , ::boost::optional<we::type::eureka_id_type>{}
                         , std::list<we::type::Preference>{}
                         )
                       );

    oar << net;
  }

  ::boost::archive::text_iarchive iar (iostream);

  we::type::net_type net;
  iar >> net;

  BOOST_REQUIRE
    ( !net.fire_expressions_and_extract_activity_random_TESTING_ONLY
        (random_engine(), unexpected_workflow_response, unexpected_eureka)
    );
}

BOOST_AUTO_TEST_CASE (transition_that_depends_on_own_output_can_fire)
{
  pnet::type::signature::signature_type const signature
    (std::string ("control"));

  we::type::net_type net;

  auto&& add_place
    ([&]() -> we::place_id_type
     {
       static int place {0};

       return net.add_place
         (place::type ( std::to_string (++place)
                      , signature
                      , ::boost::none
                      , no_properties()
                      )
         );
     }
    );

  we::place_id_type const place_in (add_place());
  we::place_id_type const place_out (add_place());
  we::place_id_type const place_credit (add_place());

  net.put_value (place_in, we::type::literal::control());
  net.put_value (place_credit, we::type::literal::control());

  we::type::Transition transition
    ( fhg::util::testing::random_identifier()
    , we::type::Expression ("${out} := ${in}")
    , ::boost::none
    , no_properties()
    , we::priority_type()
    , ::boost::optional<we::type::eureka_id_type>{}
    , std::list<we::type::Preference>{}
    );

  auto&& add_port
    ([&] ( std::string const& name
         , we::type::PortDirection direction
         ) -> we::port_id_type
     {
       return transition.add_port
         (we::type::Port (name, direction, signature, no_properties()));
     }
    );

  we::port_id_type const port_in (add_port ("in", we::type::port::direction::In{}));
  we::port_id_type const port_out (add_port ("out", we::type::port::direction::Out{}));
  we::port_id_type const port_credit_in (add_port ("c", we::type::port::direction::In{}));
  we::port_id_type const port_credit_out (add_port ("c", we::type::port::direction::Out{}));

  we::transition_id_type const transition_id (net.add_transition (transition));

  auto&& connect
    ([&] (we::edge::type edge, we::place_id_type place, we::port_id_type port)
     {
       net.add_connection (edge, transition_id, place, port, no_properties());
     }
    );

  connect (we::edge::PT{}, place_in, port_in);
  connect (we::edge::TP{}, place_out, port_out);
  connect (we::edge::PT{}, place_credit, port_credit_in);
  connect (we::edge::TP{}, place_credit, port_credit_out);

  BOOST_REQUIRE_EQUAL (net.get_token (place_in).size(), 1);
  BOOST_REQUIRE (net.get_token (place_out).empty());
  BOOST_REQUIRE_EQUAL (net.get_token (place_credit).size(), 1);

  BOOST_REQUIRE
    ( !net.fire_expressions_and_extract_activity_random_TESTING_ONLY
        (random_engine(), unexpected_workflow_response, unexpected_eureka)
    );

  BOOST_REQUIRE (net.get_token (place_in).empty());
  BOOST_REQUIRE_EQUAL (net.get_token (place_out).size(), 1);
  BOOST_REQUIRE_EQUAL (net.get_token (place_credit).size(), 1);
}

BOOST_DATA_TEST_CASE
  ( might_use_virtual_memory_is_correctly_inferred
  , ::boost::unit_test::data::make ({true, false})
  , has_memory_buffers
  )
{
  we::type::net_type net;

  we::type::Transition transition
    ( fhg::util::testing::random_string()
    , we::type::ModuleCall
        ( fhg::util::testing::random_string()
        , fhg::util::testing::random_string()
        , ( has_memory_buffers
          ? std::unordered_map<std::string, we::type::MemoryBufferInfo>
              {{fhg::util::testing::random_string(), we::type::MemoryBufferInfo()}}
          : std::unordered_map<std::string, we::type::MemoryBufferInfo> {}
          )
        , std::list<we::type::memory_transfer>()
        , std::list<we::type::memory_transfer>()
        , fhg::util::testing::random<bool>{}()
        , fhg::util::testing::random<bool>{}()
        )
    , ::boost::none
    , {}
    , we::priority_type()
    , ::boost::optional<we::type::eureka_id_type>{}
    , std::list<we::type::Preference>{}
    );

  net.add_transition (transition);

  BOOST_REQUIRE_EQUAL (net.might_use_virtual_memory(), has_memory_buffers);
}

namespace
{
  we::type::MultiModuleCall create_testing_multimodule
    (std::list<we::type::Preference> const& preferences, bool has_memory_buffers)
  {
    we::type::MultiModuleCall multimodule;

    for (auto const& target : preferences)
    {
      multimodule.emplace
        ( target
        , we::type::ModuleCall
            ( fhg::util::testing::random_string()
            , fhg::util::testing::random_string()
            , ( has_memory_buffers
              ? std::unordered_map<std::string, we::type::MemoryBufferInfo>
                  {{fhg::util::testing::random_string(), we::type::MemoryBufferInfo()}}
              : std::unordered_map<std::string, we::type::MemoryBufferInfo> {}
              )
            , std::list<we::type::memory_transfer>()
            , std::list<we::type::memory_transfer>()
            , fhg::util::testing::random<bool>{}()
            , fhg::util::testing::random<bool>{}()
            )
        );
    }

    return multimodule;
  }
}

BOOST_DATA_TEST_CASE
  ( might_use_virtual_memory_is_correctly_inferred_when_using_preferences
  , ::boost::unit_test::data::make ({true, false})
  , has_memory_buffers
  )
{
  we::type::net_type net;

  auto const preferences
    ( fhg::util::testing::unique_randoms<std::list<we::type::Preference>>
        (fhg::util::testing::random<std::size_t>{} (10, 1))
    );

  we::type::Transition transition
    ( fhg::util::testing::random_string()
    , create_testing_multimodule (preferences, has_memory_buffers)
    , ::boost::none
    , {}
    , we::priority_type()
    , ::boost::optional<we::type::eureka_id_type>{}
    , preferences
    );

  net.add_transition (transition);

  BOOST_REQUIRE_EQUAL (net.might_use_virtual_memory(), has_memory_buffers);
}

BOOST_DATA_TEST_CASE
  ( might_have_tasks_requiring_multiple_workers_is_correctly_inferred
  , ::boost::unit_test::data::make ({false, true})
  , has_tasks_requiring_multiple_workers
  )
{
  we::type::net_type net;

  we::type::property::type properties;

  if (has_tasks_requiring_multiple_workers)
  {
    properties.set
      ( {"fhg", "drts", "schedule", "num_worker"}
      , std::to_string (fhg::util::testing::random<unsigned long>{}(10, 2)) + "UL"
      );
  }

  we::type::Transition transition
    ( fhg::util::testing::random_string()
    , we::type::ModuleCall
        ( fhg::util::testing::random_string()
        , fhg::util::testing::random_string()
        , std::unordered_map<std::string, we::type::MemoryBufferInfo> {}
        , std::list<we::type::memory_transfer>()
        , std::list<we::type::memory_transfer>()
        , fhg::util::testing::random<bool>{}()
        , fhg::util::testing::random<bool>{}()
        )
    , ::boost::none
    , properties
    , we::priority_type()
    , ::boost::optional<we::type::eureka_id_type>{}
    , std::list<we::type::Preference>{}
    );

  net.add_transition (transition);

  BOOST_REQUIRE_EQUAL
    ( net.might_have_tasks_requiring_multiple_workers()
    , has_tasks_requiring_multiple_workers
    );
}

namespace we
{
  namespace type
  {
    BOOST_AUTO_TEST_CASE (empty_net)
    {
      net_type net;

      BOOST_REQUIRE (net.places().empty());
      BOOST_REQUIRE (net.transitions().empty());
      BOOST_REQUIRE (net.transition_to_place().empty());
      BOOST_REQUIRE (net.place_to_transition_consume().empty());
      BOOST_REQUIRE (net.place_to_transition_read().empty());
      BOOST_REQUIRE (net.port_to_place().empty());
      BOOST_REQUIRE (net.port_to_response().empty());
      BOOST_REQUIRE (net.place_to_port().empty());
      BOOST_REQUIRE
        ( !net.fire_expressions_and_extract_activity_random_TESTING_ONLY
            (random_engine(), unexpected_workflow_response, unexpected_eureka)
        );
    }
  }
}
