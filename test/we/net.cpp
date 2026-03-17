// Copyright (C) 2014-2016,2018,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/we/exception.hpp>
#include <gspc/we/type/Activity.hpp>
#include <gspc/we/type/Expression.hpp>
#include <gspc/we/type/MemoryBufferInfo.hpp>
#include <gspc/we/type/ModuleCall.hpp>
#include <gspc/we/type/Transition.hpp>
#include <gspc/we/type/net.hpp>
#include <gspc/we/type/shared.hpp>
#include <gspc/we/type/value/poke.hpp>
#include <gspc/we/type/value/name.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>

#include <functional>
#include <gspc/testing/random.hpp>
#include <gspc/testing/random/string.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

#include <fmt/format.h>

#include <sstream>

#include <test/we/net.common.hpp>

BOOST_AUTO_TEST_CASE (transition_without_input_port_can_not_fire)
{
  gspc::we::type::net_type net;
  net.add_transition ( gspc::we::type::Transition
                       ( gspc::testing::random_string()
                       , gspc::we::type::Expression()
                       , std::nullopt
                       , no_properties()
                       , gspc::we::priority_type()
                       , std::optional<gspc::we::type::eureka_id_type>{}
                       , std::list<gspc::we::type::Preference>{}
                       , gspc::we::type::track_shared{}
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
    gspc::we::type::net_type net;
    net.add_transition ( gspc::we::type::Transition
                         ( gspc::testing::random_string()
                         , gspc::we::type::Expression()
                         , std::nullopt
                         , no_properties()
                         , gspc::we::priority_type()
                         , std::optional<gspc::we::type::eureka_id_type>{}
                         , std::list<gspc::we::type::Preference>{}
                         , gspc::we::type::track_shared{}
                         )
                       );

    oar << net;
  }

  ::boost::archive::text_iarchive iar (iostream);

  gspc::we::type::net_type net;
  iar >> net;

  BOOST_REQUIRE
    ( !net.fire_expressions_and_extract_activity_random_TESTING_ONLY
        (random_engine(), unexpected_workflow_response, unexpected_eureka)
    );
}

BOOST_AUTO_TEST_CASE (transition_that_depends_on_own_output_can_fire)
{
  gspc::pnet::type::signature::signature_type const signature
    (std::string ("control"));

  gspc::we::type::net_type net;

  auto&& add_place
    ([&]() -> gspc::we::place_id_type
     {
       static int place {0};

       return net.add_place
         (gspc::we::type::place::type ( std::to_string (++place)
                      , signature
                      , std::nullopt
                      , std::nullopt
                      , no_properties()
                      , gspc::we::type::place::type::Generator::No{}
                      )
         );
     }
    );

  gspc::we::place_id_type const place_in (add_place());
  gspc::we::place_id_type const place_out (add_place());
  gspc::we::place_id_type const place_credit (add_place());

  net.put_value (place_in, gspc::we::type::literal::control());
  net.put_value (place_credit, gspc::we::type::literal::control());

  gspc::we::type::Transition transition
    ( gspc::testing::random_identifier()
    , gspc::we::type::Expression ("${out} := ${in}")
    , std::nullopt
    , no_properties()
    , gspc::we::priority_type()
    , std::optional<gspc::we::type::eureka_id_type>{}
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  auto&& add_port
    ([&] ( std::string const& name
         , gspc::we::type::PortDirection direction
         ) -> gspc::we::port_id_type
     {
       return transition.add_port
         (gspc::we::type::Port (name, direction, signature, no_properties()));
     }
    );

  gspc::we::port_id_type const port_in (add_port ("in", gspc::we::type::port::direction::In{}));
  gspc::we::port_id_type const port_out (add_port ("out", gspc::we::type::port::direction::Out{}));
  gspc::we::port_id_type const port_credit_in (add_port ("c", gspc::we::type::port::direction::In{}));
  gspc::we::port_id_type const port_credit_out (add_port ("c", gspc::we::type::port::direction::Out{}));

  gspc::we::transition_id_type const transition_id (net.add_transition (transition));

  auto&& connect
    ([&] (gspc::we::edge::type edge, gspc::we::place_id_type place, gspc::we::port_id_type port)
     {
       net.add_connection (edge, transition_id, place, port, no_properties());
     }
    );

  connect (gspc::we::edge::PT{}, place_in, port_in);
  connect (gspc::we::edge::TP{}, place_out, port_out);
  connect (gspc::we::edge::PT{}, place_credit, port_credit_in);
  connect (gspc::we::edge::TP{}, place_credit, port_credit_out);

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
  gspc::we::type::net_type net;

  gspc::we::type::Transition transition
    ( gspc::testing::random_string()
    , gspc::we::type::ModuleCall
        ( gspc::testing::random_string()
        , gspc::testing::random_string()
        , ( has_memory_buffers
          ? std::unordered_map<std::string, gspc::we::type::MemoryBufferInfo>
              {{gspc::testing::random_string(), gspc::we::type::MemoryBufferInfo()}}
          : std::unordered_map<std::string, gspc::we::type::MemoryBufferInfo> {}
          )
        , std::list<gspc::we::type::memory_transfer>()
        , std::list<gspc::we::type::memory_transfer>()
        , gspc::testing::random<bool>{}()
        , gspc::testing::random<bool>{}()
        )
    , std::nullopt
    , {}
    , gspc::we::priority_type()
    , std::optional<gspc::we::type::eureka_id_type>{}
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  net.add_transition (transition);

  BOOST_REQUIRE_EQUAL (net.might_use_virtual_memory(), has_memory_buffers);
}

namespace
{
  gspc::we::type::MultiModuleCall create_testing_multimodule
    (std::list<gspc::we::type::Preference> const& preferences, bool has_memory_buffers)
  {
    gspc::we::type::MultiModuleCall multimodule;

    for (auto const& target : preferences)
    {
      multimodule.emplace
        ( target
        , gspc::we::type::ModuleCall
            ( gspc::testing::random_string()
            , gspc::testing::random_string()
            , ( has_memory_buffers
              ? std::unordered_map<std::string, gspc::we::type::MemoryBufferInfo>
                  {{gspc::testing::random_string(), gspc::we::type::MemoryBufferInfo()}}
              : std::unordered_map<std::string, gspc::we::type::MemoryBufferInfo> {}
              )
            , std::list<gspc::we::type::memory_transfer>()
            , std::list<gspc::we::type::memory_transfer>()
            , gspc::testing::random<bool>{}()
            , gspc::testing::random<bool>{}()
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
  gspc::we::type::net_type net;

  auto const preferences
    ( gspc::testing::unique_randoms<std::list<gspc::we::type::Preference>>
        (gspc::testing::random<std::size_t>{} (10, 1))
    );

  gspc::we::type::Transition transition
    ( gspc::testing::random_string()
    , create_testing_multimodule (preferences, has_memory_buffers)
    , std::nullopt
    , {}
    , gspc::we::priority_type()
    , std::optional<gspc::we::type::eureka_id_type>{}
    , preferences
    , gspc::we::type::track_shared{}
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
  gspc::we::type::net_type net;

  gspc::we::type::property::type properties;

  if (has_tasks_requiring_multiple_workers)
  {
    properties.set
      ( {"fhg", "drts", "schedule", "num_worker"}
      , std::to_string (gspc::testing::random<unsigned long>{}(10, 2)) + "UL"
      );
  }

  gspc::we::type::Transition transition
    ( gspc::testing::random_string()
    , gspc::we::type::ModuleCall
        ( gspc::testing::random_string()
        , gspc::testing::random_string()
        , std::unordered_map<std::string, gspc::we::type::MemoryBufferInfo> {}
        , std::list<gspc::we::type::memory_transfer>()
        , std::list<gspc::we::type::memory_transfer>()
        , gspc::testing::random<bool>{}()
        , gspc::testing::random<bool>{}()
        )
    , std::nullopt
    , properties
    , gspc::we::priority_type()
    , std::optional<gspc::we::type::eureka_id_type>{}
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  net.add_transition (transition);

  BOOST_REQUIRE_EQUAL
    ( net.might_have_tasks_requiring_multiple_workers()
    , has_tasks_requiring_multiple_workers
    );
}


  namespace gspc::we::type
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

    namespace
    {
      // Unified fixture for testing shared cleanup with different
      // container types. The data_type parameter specifies the type
      // of the data place (e.g., "shared_cleanup", "list", "set", "map").
      struct simple_shared_cleanup_fixture
      {
        net_type net;
        std::string const data_type;

        gspc::we::place_id_type const cleanup_place_id
          { net.add_place
            ( gspc::we::type::place::type
              ( "cleanup"
              , std::string {"int"}
              , std::nullopt
              , true  // shared_sink = true
              , no_properties()
              , gspc::we::type::place::type::Generator::No{}
              )
            )
          };
        gspc::we::place_id_type data_place_id;

        explicit simple_shared_cleanup_fixture (std::string data_type_arg)
          : data_type {std::move (data_type_arg)}
          , data_place_id
            { net.add_place
              ( gspc::we::type::place::type
                ( "data"
                , data_type
                , std::nullopt
                , std::nullopt
                , no_properties()
                , gspc::we::type::place::type::Generator::No{}
                )
              )
            }
        {
          auto transition
            { Transition
              { "consume"
              , Expression {""}
              , std::nullopt
              , no_properties()
              , gspc::we::priority_type()
              , std::optional<gspc::we::type::eureka_id_type>{}
              , std::list<gspc::we::type::Preference>{}
              , {.input = true, .output = false}
              }
            };

          auto const port_in
            { transition.add_port
              ( Port
                ( "in"
                , port::direction::In{}
                , data_type
                , no_properties()
                )
              )
            };

          net.add_connection
            ( edge::PT{}
            , net.add_transition (transition)
            , data_place_id
            , port_in
            , no_properties()
            );
        }

        void fire (gspc::pnet::type::value::value_type const& value)
        {
          net.put_value (data_place_id, value);

          BOOST_REQUIRE_EQUAL (net.get_token (data_place_id).size(), 1);
          BOOST_REQUIRE (net.get_token (cleanup_place_id).empty());

          net.fire_expressions_and_extract_activity_random_TESTING_ONLY
            ( random_engine()
            , unexpected_workflow_response
            , unexpected_eureka
            );
        }
      };

      // Convenience fixture for direct shared value tests
      struct shared_cleanup_test_fixture : simple_shared_cleanup_fixture
      {
        shared_cleanup_test_fixture()
          : simple_shared_cleanup_fixture {"shared_cleanup"}
        {}
      };
    }

    BOOST_FIXTURE_TEST_CASE
      ( shared_cleanup_with_wrong_wrapped_type_throws
      , shared_cleanup_test_fixture
      )
    {
      auto const inner_value
        {std::invoke (gspc::testing::random<long>{})};

      gspc::testing::require_exception
        ( [&]
          {
            fire (gspc::we::type::shared {inner_value, "cleanup"});
          }
        , gspc::pnet::exception::type_mismatch
          { std::string {"int"}
          , gspc::pnet::type::value::value_type {inner_value}
          , {"cleanup"}
          }
        );
    }

    BOOST_FIXTURE_TEST_CASE
      ( shared_cleanup_with_correct_wrapped_type_succeeds
      , shared_cleanup_test_fixture
      )
    {
      auto const inner_value
        {std::invoke (gspc::testing::random<int>{})};

      BOOST_CHECK_NO_THROW
        (fire (gspc::we::type::shared {inner_value, "cleanup"}));

      BOOST_REQUIRE_EQUAL (net.get_token (cleanup_place_id).size(), 1);

      BOOST_CHECK_EQUAL
        ( ::boost::get<int> (net.get_token (cleanup_place_id).begin()->second)
        , inner_value
        );
    }

    namespace
    {
      // Convenience fixture for list container tests
      struct nested_shared_in_list_fixture : simple_shared_cleanup_fixture
      {
        nested_shared_in_list_fixture()
          : simple_shared_cleanup_fixture {"list"}
        {}
      };
    }

    BOOST_FIXTURE_TEST_CASE
      ( nested_shared_in_list_triggers_cleanup
      , nested_shared_in_list_fixture
      )
    {
      auto const inner_value
        {std::invoke (gspc::testing::random<int>{})};

      BOOST_CHECK_NO_THROW
        ( fire
            ( std::list<gspc::pnet::type::value::value_type>
                {gspc::we::type::shared {inner_value, "cleanup"}}
            )
        );

      // The nested shared should have triggered cleanup
      BOOST_REQUIRE_EQUAL (net.get_token (cleanup_place_id).size(), 1);

      BOOST_CHECK_EQUAL
        ( ::boost::get<int> (net.get_token (cleanup_place_id).begin()->second)
        , inner_value
        );
    }

    BOOST_FIXTURE_TEST_CASE
      ( multiple_nested_shared_in_list_all_trigger_cleanup
      , nested_shared_in_list_fixture
      )
    {
      auto random_int {gspc::testing::random<int>{}};
      auto const value1 {std::invoke (random_int)};
      auto const value2 {std::invoke (random_int)};
      auto const value3 {std::invoke (random_int)};

      BOOST_CHECK_NO_THROW
        ( fire
            ( std::list<gspc::pnet::type::value::value_type>
                { gspc::we::type::shared {value1, "cleanup"}
                , gspc::we::type::shared {value2, "cleanup"}
                , gspc::we::type::shared {value3, "cleanup"}
                }
            )
        );

      // All 3 nested shareds should have triggered cleanup
      BOOST_REQUIRE_EQUAL (net.get_token (cleanup_place_id).size(), 3);

      // Collect all cleanup values
      auto cleanup_values {std::set<int>{}};
      for (auto const& [_, token] : net.get_token (cleanup_place_id))
      {
        cleanup_values.insert (::boost::get<int> (token));
      }

      BOOST_CHECK (cleanup_values.count (value1));
      BOOST_CHECK (cleanup_values.count (value2));
      BOOST_CHECK (cleanup_values.count (value3));
    }

    namespace
    {
      struct nested_shared_in_struct_test_fixture
      {
        net_type net;

        gspc::we::place_id_type const cleanup_place_id
          { net.add_place
            ( gspc::we::type::place::type
              ( "cleanup"
              , std::string {"int"}
              , std::nullopt
              , true  // shared_sink = true
              , no_properties()
              , gspc::we::type::place::type::Generator::No{}
              )
            )
          };
        // Struct signature for the data place:
        // struct with regular_field (long) and shared_field (shared_cleanup)
        gspc::pnet::type::signature::structure_type const struct_fields
          { std::make_pair
              (std::string {"regular_field"}, std::string {"long"})
          , std::make_pair
              (std::string {"shared_field"}, std::string {"shared_cleanup"})
          };
        gspc::pnet::type::signature::signature_type const struct_sig
          { gspc::pnet::type::signature::structured_type
            { std::make_pair (std::string {"data_struct"}, struct_fields)
            }
          };

        gspc::we::place_id_type const data_place_id
          { net.add_place
            ( gspc::we::type::place::type
              ( "data"
              , struct_sig
              , std::nullopt
              , std::nullopt
              , no_properties()
              , gspc::we::type::place::type::Generator::No{}
              )
            )
          };

        nested_shared_in_struct_test_fixture()
        {
          auto transition
            { Transition
              { "consume"
              , Expression {""}
              , std::nullopt
              , no_properties()
              , gspc::we::priority_type()
              , std::optional<gspc::we::type::eureka_id_type>{}
              , std::list<gspc::we::type::Preference>{}
              , {.input = true, .output = false}
              }
            };

          auto const port_in
            { transition.add_port
              ( Port
                ( "in"
                , port::direction::In{}
                , struct_sig
                , no_properties()
                )
              )
            };

          net.add_connection
            ( edge::PT{}
            , net.add_transition (transition)
            , data_place_id
            , port_in
            , no_properties()
            );
        }

        void fire (gspc::pnet::type::value::value_type const& struct_value)
        {
          net.put_value (data_place_id, struct_value);

          BOOST_REQUIRE_EQUAL (net.get_token (data_place_id).size(), 1);
          BOOST_REQUIRE (net.get_token (cleanup_place_id).empty());

          net.fire_expressions_and_extract_activity_random_TESTING_ONLY
            ( random_engine()
            , unexpected_workflow_response
            , unexpected_eureka
            );
        }
      };
    }

    BOOST_FIXTURE_TEST_CASE
      ( nested_shared_in_struct_field_triggers_cleanup
      , nested_shared_in_struct_test_fixture
      )
    {
      using gspc::pnet::type::value::poke;

      auto const inner_value
        {std::invoke (gspc::testing::random<int>{})};
      auto const other_field_value
        {std::invoke (gspc::testing::random<long>{})};

      // Create a struct where one field is a shared value
      auto struct_with_shared {gspc::pnet::type::value::value_type{}};
      poke ("regular_field", struct_with_shared, other_field_value);
      poke ( "shared_field"
           , struct_with_shared
           , gspc::we::type::shared {inner_value, "cleanup"}
           );

      BOOST_CHECK_NO_THROW (fire (struct_with_shared));

      // The nested shared in the struct field should have triggered cleanup
      BOOST_REQUIRE_EQUAL (net.get_token (cleanup_place_id).size(), 1);

      BOOST_CHECK_EQUAL
        ( ::boost::get<int> (net.get_token (cleanup_place_id).begin()->second)
        , inner_value
        );
    }

    namespace
    {
      // Test fixture for shared value wrapping a struct that
      // contains another shared
      struct shared_of_struct_with_shared_test_fixture
      {
        net_type net;

        // Struct signature for the struct wrapped by the outer shared
        // Fields: regular_field (long), shared_field (shared_inner_cleanup)
        gspc::pnet::type::signature::structure_type const outer_struct_fields
          { std::make_pair
              (std::string {"regular_field"}, std::string {"long"})
          , std::make_pair
              ( std::string {"shared_field"}
              , std::string {"shared_inner_cleanup"}
              )
          };
        gspc::pnet::type::signature::signature_type const outer_struct_sig
          { gspc::pnet::type::signature::structured_type
            { std::make_pair
                (std::string {"outer_struct"}, outer_struct_fields)
            }
          };

        // Cleanup place for the outer shared (receives the struct)
        gspc::we::place_id_type const outer_cleanup_place_id
          { net.add_place
            ( gspc::we::type::place::type
              ( "outer_cleanup"
              , outer_struct_sig
              , std::nullopt
              , true  // shared_sink = true
              , no_properties()
              , gspc::we::type::place::type::Generator::No{}
              )
            )
          };
        // Cleanup place for the inner shared field (receives the int)
        gspc::we::place_id_type const inner_cleanup_place_id
          { net.add_place
            ( gspc::we::type::place::type
              ( "inner_cleanup"
              , std::string {"int"}
              , std::nullopt
              , true  // shared_sink = true
              , no_properties()
              , gspc::we::type::place::type::Generator::No{}
              )
            )
          };
        gspc::we::place_id_type const data_place_id
          { net.add_place
            ( gspc::we::type::place::type
              ( "data"
              // Shared wrapping a struct
              , std::string {"shared_outer_cleanup"}
              , std::nullopt
              , std::nullopt
              , no_properties()
              , gspc::we::type::place::type::Generator::No{}
              )
            )
          };

        shared_of_struct_with_shared_test_fixture()
        {
          auto transition
            { Transition
              { "consume"
              , Expression {""}
              , std::nullopt
              , no_properties()
              , gspc::we::priority_type()
              , std::optional<gspc::we::type::eureka_id_type>{}
              , std::list<gspc::we::type::Preference>{}
              , {.input = true, .output = false}
              }
            };

          auto const port_in
            { transition.add_port
              ( Port
                ( "in"
                , port::direction::In{}
                , std::string {"shared_outer_cleanup"}
                , no_properties()
                )
              )
            };

          net.add_connection
            ( edge::PT{}
            , net.add_transition (transition)
            , data_place_id
            , port_in
            , no_properties()
            );
        }

        void fire (gspc::pnet::type::value::value_type const& shared_value)
        {
          net.put_value (data_place_id, shared_value);

          BOOST_REQUIRE_EQUAL (net.get_token (data_place_id).size(), 1);
          BOOST_REQUIRE (net.get_token (outer_cleanup_place_id).empty());
          BOOST_REQUIRE (net.get_token (inner_cleanup_place_id).empty());

          net.fire_expressions_and_extract_activity_random_TESTING_ONLY
            ( random_engine()
            , unexpected_workflow_response
            , unexpected_eureka
            );
        }
      };
    }

    BOOST_FIXTURE_TEST_CASE
      ( shared_struct_containing_shared_field_nests_cleanup
      , shared_of_struct_with_shared_test_fixture
      )
    {
      using gspc::pnet::type::value::value_type;
      using gspc::pnet::type::value::poke;
      using gspc::pnet::type::value::structured_type;

      auto const inner_value
        {std::invoke (gspc::testing::random<int>{})};
      auto const other_field_value
        {std::invoke (gspc::testing::random<long>{})};

      // Create a struct with a shared field, wrap in outer shared, and fire
      auto inner_struct {value_type{}};
      poke ("regular_field", inner_struct, other_field_value);
      poke ( "shared_field"
           , inner_struct
           , gspc::we::type::shared {inner_value, "inner_cleanup"}
           );

      BOOST_CHECK_NO_THROW
        ( fire (gspc::we::type::shared {inner_struct, "outer_cleanup"})
        );

      // The outer shared's cleanup should have been triggered
      // (placing the struct on outer_cleanup)
      BOOST_REQUIRE_EQUAL (net.get_token (outer_cleanup_place_id).size(), 1);

      // With nested cleanup, the inner shared field within the struct IS
      // automatically cleaned up when the outer shared is consumed.
      // The inner value (int) is placed on inner_cleanup.
      BOOST_REQUIRE_EQUAL (net.get_token (inner_cleanup_place_id).size(), 1);

      // Verify the inner cleanup value - it should be the int value
      BOOST_CHECK_EQUAL
        ( ::boost::get<int>
            (net.get_token (inner_cleanup_place_id).begin()->second)
        , inner_value
        );

      // Verify the outer cleanup value - it should be the struct
      auto const& placed_struct
        { ::boost::get<structured_type>
            (net.get_token (outer_cleanup_place_id).begin()->second)
        };
      // The struct should have both fields
      BOOST_CHECK_EQUAL (placed_struct.size(), 2);

      // The shared field should still contain the inner shared value
      // (it remains as-is in the struct, but its cleanup was triggered)
      auto found_shared_field {false};
      for (auto const& [field_name, field_value] : placed_struct)
      {
        if (field_name == "shared_field")
        {
          found_shared_field = true;
          // The field should still be a shared value
          BOOST_CHECK
            (::boost::get<gspc::we::type::shared> (&field_value) != nullptr);
        }
      }
      BOOST_CHECK (found_shared_field);
    }

    namespace
    {
      // Convenience fixture for set container tests
      struct nested_shared_in_set_fixture : simple_shared_cleanup_fixture
      {
        nested_shared_in_set_fixture()
          : simple_shared_cleanup_fixture {"set"}
        {}
      };
    }

    BOOST_FIXTURE_TEST_CASE
      ( nested_shared_in_set_triggers_cleanup
      , nested_shared_in_set_fixture
      )
    {
      auto const inner_value
        {std::invoke (gspc::testing::random<int>{})};

      BOOST_CHECK_NO_THROW
        ( fire
            ( std::set<gspc::pnet::type::value::value_type>
                {gspc::we::type::shared {inner_value, "cleanup"}}
            )
        );

      // The nested shared should have triggered cleanup
      BOOST_REQUIRE_EQUAL (net.get_token (cleanup_place_id).size(), 1);

      BOOST_CHECK_EQUAL
        ( ::boost::get<int> (net.get_token (cleanup_place_id).begin()->second)
        , inner_value
        );
    }

    BOOST_FIXTURE_TEST_CASE
      ( multiple_nested_shared_in_set_all_trigger_cleanup
      , nested_shared_in_set_fixture
      )
    {
      auto random_int {gspc::testing::random<int>{}};
      auto const value1 {std::invoke (random_int)};
      auto const value2 {std::invoke (random_int)};
      auto const value3 {std::invoke (random_int)};

      BOOST_CHECK_NO_THROW
        ( fire
            ( std::set<gspc::pnet::type::value::value_type>
                { gspc::we::type::shared {value1, "cleanup"}
                , gspc::we::type::shared {value2, "cleanup"}
                , gspc::we::type::shared {value3, "cleanup"}
                }
            )
        );

      // All 3 nested shareds should have triggered cleanup
      BOOST_REQUIRE_EQUAL (net.get_token (cleanup_place_id).size(), 3);

      // Collect all cleanup values
      auto cleanup_values {std::set<int>{}};
      for (auto const& [_, token] : net.get_token (cleanup_place_id))
      {
        cleanup_values.insert (::boost::get<int> (token));
      }

      BOOST_CHECK (cleanup_values.count (value1));
      BOOST_CHECK (cleanup_values.count (value2));
      BOOST_CHECK (cleanup_values.count (value3));
    }

    namespace
    {
      // Convenience fixture for map container tests
      struct nested_shared_in_map_fixture : simple_shared_cleanup_fixture
      {
        nested_shared_in_map_fixture()
          : simple_shared_cleanup_fixture {"map"}
        {}
      };
    }

    BOOST_FIXTURE_TEST_CASE
      ( nested_shared_in_map_value_triggers_cleanup
      , nested_shared_in_map_fixture
      )
    {
      using gspc::pnet::type::value::value_type;

      auto const inner_value
        {std::invoke (gspc::testing::random<int>{})};

      BOOST_CHECK_NO_THROW
        ( fire
            ( std::map<value_type, value_type>
                { { std::string {"key1"}
                  , gspc::we::type::shared {inner_value, "cleanup"}
                  }
                }
            )
        );

      // The nested shared should have triggered cleanup
      BOOST_REQUIRE_EQUAL (net.get_token (cleanup_place_id).size(), 1);

      BOOST_CHECK_EQUAL
        ( ::boost::get<int> (net.get_token (cleanup_place_id).begin()->second)
        , inner_value
        );
    }

    BOOST_FIXTURE_TEST_CASE
      ( nested_shared_in_map_key_triggers_cleanup
      , nested_shared_in_map_fixture
      )
    {
      using gspc::pnet::type::value::value_type;

      auto const inner_value
        {std::invoke (gspc::testing::random<int>{})};

      BOOST_CHECK_NO_THROW
        ( fire
            ( std::map<value_type, value_type>
                { { gspc::we::type::shared {inner_value, "cleanup"}
                  , std::string {"value1"}
                  }
                }
            )
        );

      // The nested shared in the key should have triggered cleanup
      BOOST_REQUIRE_EQUAL (net.get_token (cleanup_place_id).size(), 1);

      BOOST_CHECK_EQUAL
        ( ::boost::get<int> (net.get_token (cleanup_place_id).begin()->second)
        , inner_value
        );
    }

    BOOST_FIXTURE_TEST_CASE
      ( nested_shared_in_map_key_and_value_both_trigger_cleanup
      , nested_shared_in_map_fixture
      )
    {
      using gspc::pnet::type::value::value_type;

      auto random_int {gspc::testing::random<int>{}};
      auto const key_value {std::invoke (random_int)};
      auto const val_value {std::invoke (random_int)};

      BOOST_CHECK_NO_THROW
        ( fire
            ( std::map<value_type, value_type>
                { { gspc::we::type::shared {key_value, "cleanup"}
                  , gspc::we::type::shared {val_value, "cleanup"}
                  }
                }
            )
        );

      // Both the key and value shared should have triggered cleanup
      BOOST_REQUIRE_EQUAL (net.get_token (cleanup_place_id).size(), 2);

      // Collect all cleanup values
      auto cleanup_values {std::set<int>{}};
      for (auto const& [_, token] : net.get_token (cleanup_place_id))
      {
        cleanup_values.insert (::boost::get<int> (token));
      }

      BOOST_CHECK (cleanup_values.count (key_value));
      BOOST_CHECK (cleanup_values.count (val_value));
    }
  }

  // Tests for shared value comparison operators
  BOOST_AUTO_TEST_CASE (shared_equality_same_value_same_place)
  {
    auto const value {std::invoke (gspc::testing::random<int>{})};

    gspc::we::type::shared const s1 {value, "cleanup"};
    gspc::we::type::shared const s2 {value, "cleanup"};

    BOOST_CHECK (s1 == s2);
    BOOST_CHECK (!(s1 != s2));
  }

  BOOST_AUTO_TEST_CASE (shared_equality_different_value_same_place)
  {
    auto const value1 {std::invoke (gspc::testing::random<int>{})};
    auto const value2 {value1 + 1};  // Ensure different

    gspc::we::type::shared const s1 {value1, "cleanup"};
    gspc::we::type::shared const s2 {value2, "cleanup"};

    BOOST_CHECK (!(s1 == s2));
    BOOST_CHECK (s1 != s2);
  }

  BOOST_AUTO_TEST_CASE (shared_equality_same_value_different_place)
  {
    auto const value {std::invoke (gspc::testing::random<int>{})};

    gspc::we::type::shared const s1 {value, "cleanup1"};
    gspc::we::type::shared const s2 {value, "cleanup2"};

    BOOST_CHECK (!(s1 == s2));
    BOOST_CHECK (s1 != s2);
  }

  BOOST_AUTO_TEST_CASE (shared_less_than_by_place_name)
  {
    auto const value {std::invoke (gspc::testing::random<int>{})};

    gspc::we::type::shared const s1 {value, "aaa"};
    gspc::we::type::shared const s2 {value, "zzz"};

    BOOST_CHECK (s1 < s2);
    BOOST_CHECK (!(s2 < s1));
    BOOST_CHECK (s1 <= s2);
    BOOST_CHECK (s2 > s1);
    BOOST_CHECK (s2 >= s1);
  }

  BOOST_AUTO_TEST_CASE (shared_less_than_by_value_when_same_place)
  {
    gspc::we::type::shared const s1 {1, "cleanup"};
    gspc::we::type::shared const s2 {2, "cleanup"};

    BOOST_CHECK (s1 < s2);
    BOOST_CHECK (!(s2 < s1));
    BOOST_CHECK (s1 <= s2);
    BOOST_CHECK (s2 > s1);
    BOOST_CHECK (s2 >= s1);
  }

  BOOST_AUTO_TEST_CASE (shared_less_equal_when_equal)
  {
    auto const value {gspc::testing::random<int>()()};

    gspc::we::type::shared const s1 {value, "cleanup"};
    gspc::we::type::shared const s2 {value, "cleanup"};

    BOOST_CHECK (s1 <= s2);
    BOOST_CHECK (s2 <= s1);
    BOOST_CHECK (s1 >= s2);
    BOOST_CHECK (s2 >= s1);
    BOOST_CHECK (!(s1 < s2));
    BOOST_CHECK (!(s2 < s1));
  }

  // Tests for shared tracking annotation behavior
  namespace
  {
    // Fixture for testing shared tracking annotation scenarios
    struct shared_tracking_annotation_fixture
    {
      gspc::we::type::net_type net;

      gspc::we::place_id_type const cleanup_place_id
        { net.add_place
          ( gspc::we::type::place::type
            ( "cleanup"
            , std::string {"int"}
            , std::nullopt
            , true  // shared_sink
            , no_properties()
            , gspc::we::type::place::type::Generator::No{}
            )
          )
        };

      gspc::we::place_id_type const input_place_id
        { net.add_place
          ( gspc::we::type::place::type
            ( "input"
            , std::string {"shared_cleanup"}
            , std::nullopt
            , std::nullopt
            , no_properties()
            , gspc::we::type::place::type::Generator::No{}
            )
          )
        };

      gspc::we::place_id_type const output_place_id
        { net.add_place
          ( gspc::we::type::place::type
            ( "output"
            , std::string {"shared_cleanup"}
            , std::nullopt
            , std::nullopt
            , no_properties()
            , gspc::we::type::place::type::Generator::No{}
            )
          )
        };

      // Add a passthrough transition with specific tracking flags
      void add_passthrough_transition
        ( bool track_input
        , bool track_output
        )
      {
        gspc::we::type::Transition transition
          { "passthrough"
          , gspc::we::type::Expression {"${out} := ${in}"}
          , std::nullopt
          , no_properties()
          , gspc::we::priority_type()
          , std::optional<gspc::we::type::eureka_id_type>{}
          , std::list<gspc::we::type::Preference>{}
          , {.input = track_input, .output = track_output}
          };

        auto const port_in
          { transition.add_port
            ( gspc::we::type::Port
              ( "in"
              , gspc::we::type::port::direction::In{}
              , std::string {"shared_cleanup"}
              , no_properties()
              )
            )
          };

        auto const port_out
          { transition.add_port
            ( gspc::we::type::Port
              ( "out"
              , gspc::we::type::port::direction::Out{}
              , std::string {"shared_cleanup"}
              , no_properties()
              )
            )
          };

        auto const tid {net.add_transition (transition)};

        net.add_connection
          ( gspc::we::edge::PT{}
          , tid
          , input_place_id
          , port_in
          , no_properties()
          );

        net.add_connection
          ( gspc::we::edge::TP{}
          , tid
          , output_place_id
          , port_out
          , no_properties()
          );
      }
    };
  }

  // Test: Passthrough with tracking disabled (passthrough_shared="true")
  // The reference count should remain unchanged (delta=0), cleanup doesn't
  // trigger
  BOOST_FIXTURE_TEST_CASE
    ( passthrough_shared_annotation_skips_tracking
    , shared_tracking_annotation_fixture
    )
  {
    // With passthrough_shared="true", both input and output tracking
    // are disabled
    add_passthrough_transition (false, false);

    auto const inner_value {std::invoke (gspc::testing::random<int>{})};
    auto const shared_val = gspc::we::type::shared {inner_value, "cleanup"};

    // Put the shared value (this increments the counter)
    net.put_value (input_place_id, shared_val);

    // Fire the transition (no tracking, just moves the token)
    net.fire_expressions_and_extract_activity_random_TESTING_ONLY
      ( random_engine()
      , unexpected_workflow_response
      , unexpected_eureka
      );

    // Value moved to output, counter is still 1
    // (not decremented then incremented)
    BOOST_CHECK (net.get_token (input_place_id).empty());
    BOOST_CHECK_EQUAL (net.get_token (output_place_id).size(), 1);
    // Cleanup hasn't triggered because count is still 1
    BOOST_CHECK (net.get_token (cleanup_place_id).empty());
  }

  // Test: Consume without tracking
  // (consume_shared="false" or missing annotation)
  // Memory leak: reference count not decremented
  BOOST_FIXTURE_TEST_CASE
    ( missing_consume_annotation_causes_leak
    , shared_tracking_annotation_fixture
    )
  {
    // Only output tracking, input tracking disabled (incorrect if consuming)
    add_passthrough_transition (false, true);

    auto const inner_value {std::invoke (gspc::testing::random<int>{})};
    auto const shared_val = gspc::we::type::shared {inner_value, "cleanup"};

    // Put the shared value
    net.put_value (input_place_id, shared_val);

    // Fire the transition
    net.fire_expressions_and_extract_activity_random_TESTING_ONLY
      ( random_engine()
      , unexpected_workflow_response
      , unexpected_eureka
      );

    // Value moved to output, but count is now 2
    // (put +1, no -1 on consume, +1 on produce)
    BOOST_CHECK (net.get_token (input_place_id).empty());
    BOOST_CHECK_EQUAL (net.get_token (output_place_id).size(), 1);
    // Cleanup hasn't triggered - this is a memory leak!
    BOOST_CHECK (net.get_token (cleanup_place_id).empty());
  }

  // Test: Produce without tracking
  // (produce_shared="false" or missing annotation)
  // Underflow: reference count not incremented, will underflow when consumed
  BOOST_FIXTURE_TEST_CASE
    ( missing_produce_annotation_causes_underflow
    , shared_tracking_annotation_fixture
    )
  {
    // Only input tracking, output tracking disabled (incorrect if producing)
    add_passthrough_transition (true, false);

    auto const inner_value {std::invoke (gspc::testing::random<int>{})};
    auto const shared_val = gspc::we::type::shared {inner_value, "cleanup"};

    // Put the shared value (count = 1)
    net.put_value (input_place_id, shared_val);

    // Fire the transition (count goes: -1 for consume, no +1 for produce = 0)
    // Cleanup triggers immediately!
    net.fire_expressions_and_extract_activity_random_TESTING_ONLY
      ( random_engine()
      , unexpected_workflow_response
      , unexpected_eureka
      );

    // Value moved to output
    BOOST_CHECK (net.get_token (input_place_id).empty());
    BOOST_CHECK_EQUAL (net.get_token (output_place_id).size(), 1);
    // Cleanup triggered prematurely
    // - this is incorrect behavior for passthrough!
    BOOST_CHECK_EQUAL (net.get_token (cleanup_place_id).size(), 1);
  }

  // Test: Runtime error when cleanup place doesn't exist
  BOOST_AUTO_TEST_CASE (shared_cleanup_place_not_found_throws)
  {
    gspc::we::type::net_type net;

    // Create an input place with shared type pointing to non-existent cleanup
    auto const input_place_id
      { net.add_place
        ( gspc::we::type::place::type
          ( "input"
          , std::string {"shared_nonexistent"}
          , std::nullopt
          , std::nullopt
          , no_properties()
          , gspc::we::type::place::type::Generator::No{}
          )
        )
      };

    // Put a shared value - should throw because "nonexistent" place
    // doesn't exist
    auto const inner_value {std::invoke (gspc::testing::random<int>{})};
    gspc::testing::require_exception
      ( [&]
        {
          net.put_value
            ( input_place_id
            , gspc::we::type::shared {inner_value, "nonexistent"}
            );
        }
      , std::runtime_error
          { "shared cleanup place 'nonexistent' does not exist or is not"
            " marked shared_sink"
          }
      );
  }

  // Test: Runtime error when cleanup place exists but isn't marked shared_sink
  BOOST_AUTO_TEST_CASE (shared_cleanup_place_not_shared_sink_throws)
  {
    gspc::we::type::net_type net;

    // Create a place that is NOT marked as shared_sink
    net.add_place
      ( gspc::we::type::place::type
        ( "not_a_sink"
        , std::string {"int"}
        , std::nullopt
        , std::nullopt  // NOT shared_sink
        , no_properties()
        , gspc::we::type::place::type::Generator::No{}
        )
      );

    auto const input_place_id
      { net.add_place
        ( gspc::we::type::place::type
          ( "input"
          , std::string {"shared_not_a_sink"}
          , std::nullopt
          , std::nullopt
          , no_properties()
          , gspc::we::type::place::type::Generator::No{}
          )
        )
      };

    // Put a shared value targeting the non-sink place - should throw
    auto const inner_value {std::invoke (gspc::testing::random<int>{})};
    gspc::testing::require_exception
      ( [&]
        {
          net.put_value
            ( input_place_id
            , gspc::we::type::shared {inner_value, "not_a_sink"}
            );
        }
      , std::runtime_error
          { "shared cleanup place 'not_a_sink' does not exist or is not"
            " marked shared_sink"
          }
      );
  }

  // Test: Correct passthrough behavior with both tracking enabled
  BOOST_FIXTURE_TEST_CASE
    ( correct_passthrough_with_both_tracking_enabled
    , shared_tracking_annotation_fixture
    )
  {
    // Both input and output tracking enabled - correct for passthrough
    add_passthrough_transition (true, true);

    auto const inner_value {std::invoke (gspc::testing::random<int>{})};
    auto const shared_val = gspc::we::type::shared {inner_value, "cleanup"};

    // Put the shared value (count = 1)
    net.put_value (input_place_id, shared_val);

    // Fire the transition (count goes: -1 + 1 = still 1)
    net.fire_expressions_and_extract_activity_random_TESTING_ONLY
      ( random_engine()
      , unexpected_workflow_response
      , unexpected_eureka
      );

    // Value moved to output
    BOOST_CHECK (net.get_token (input_place_id).empty());
    BOOST_CHECK_EQUAL (net.get_token (output_place_id).size(), 1);
    // Cleanup hasn't triggered (count is still 1)
    BOOST_CHECK (net.get_token (cleanup_place_id).empty());
  }

BOOST_AUTO_TEST_CASE (generator_place_overflow_throws)
{
  auto const place_name
    { gspc::testing::random_identifier_without_leading_underscore()
    };

  gspc::pnet::type::signature::signature_type const signature
    { gspc::pnet::type::value::CHAR()
    };

  gspc::we::type::net_type net;

  auto const place_id
    { net.add_place
      ( gspc::we::type::place::type
        ( place_name
        , signature
        , std::nullopt
        , std::nullopt
        , no_properties()
        , gspc::we::type::place::type::Generator::Yes{}
        )
      )
    };

  auto transition
    { gspc::we::type::Transition
      ( gspc::testing::random_identifier()
      , gspc::we::type::Expression{}
      , std::nullopt
      , no_properties()
      , gspc::we::priority_type()
      , std::optional<gspc::we::type::eureka_id_type>{}
      , std::list<gspc::we::type::Preference>{}
      , gspc::we::type::track_shared{}
      )
    };

  auto const port_in
    { transition.add_port
      ( gspc::we::type::Port
        ( "in"
        , gspc::we::type::port::direction::In{}
        , signature
        , no_properties()
        )
      )
    };

  auto const tid {net.add_transition (transition)};

  net.add_connection
    ( gspc::we::edge::PT{}
    , tid
    , place_id
    , port_in
    , no_properties()
    );

  // The expression transition is always enabled (generator
  // regenerates after each firing). A single call to fire_expressions
  // fires all expression transitions in a loop until none are
  // enabled, exhausting the char domain and triggering overflow.
  //
  gspc::testing::require_exception
    ( [&]
      {
        net.fire_expressions_and_extract_activity_random_TESTING_ONLY
          ( random_engine()
          , unexpected_workflow_response
          , unexpected_eureka
          );
      }
    , gspc::pnet::exception::generator_place_overflow
        {place_name, gspc::pnet::type::value::CHAR()}
    );
}

BOOST_AUTO_TEST_CASE (add_generator_place_with_unsupported_type_throws)
{
  auto const name
    { gspc::testing::random_identifier_without_leading_underscore()
    };

  auto const random_string_that_is_not_a_known_type
    { []
      {
        auto const is_known_generator_type
          { [] (auto value)
            {
              namespace TYPE =  gspc::pnet::type::value;

              return value == TYPE::INT()
                  || value == TYPE::LONG()
                  || value == TYPE::UINT()
                  || value == TYPE::ULONG()
                  || value == TYPE::CHAR()
                  || value == TYPE::STRING()
                  || value == TYPE::BIGINT()
                  ;
            }
          };

      GENERATE:
        if ( auto value {gspc::testing::random_string()}
           ; !is_known_generator_type (value)
           )
        {
          return value;
        }

        goto GENERATE;
      }
    };

  auto const type {random_string_that_is_not_a_known_type()};

  gspc::testing::require_exception
    ( [&]
      {
        gspc::we::type::net_type{}.add_place
          ( gspc::we::type::place::type
            ( name
            , type
            , std::nullopt
            , std::nullopt
            , no_properties()
            , gspc::we::type::place::type::Generator::Yes{}
            )
          );
      }
    , std::logic_error
        (fmt::format ("generator place '{}' with unknown type '{}'", name, type))
    );
}
