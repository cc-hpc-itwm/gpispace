// Copyright (C) 2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/source_directory.hpp>

#include <gspc/we/type/value.hpp>
#include <gspc/testing/printer/we/type/value.hpp>
#include <gspc/we/type/value/poke.hpp>
#include <gspc/we/type/shared.hpp>

#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/printer/set.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>

#include <gspc/testing/random.hpp>

#include <list>
#include <functional>
#include <map>
#include <set>
#include <string>

namespace
{
  // Common test helper that captures the shared test pattern. Builds
  // and runs a workflow, then verifies the outputs match expected
  // values.
  void run_shared_test
    ( std::string const& test_name
    , std::string const& network_name
    , std::multimap<std::string, gspc::pnet::type::value::value_type> const& inputs
    , std::multimap<std::string, gspc::pnet::type::value::value_type> const& expected
    )
  {
    ::boost::program_options::options_description options_description;

    options_description.add (gspc::testing::options::source_directory());
    options_description.add (gspc::testing::options::shared_directory());
    options_description.add (gspc::options::installation());
    options_description.add (gspc::options::drts());
    options_description.add (gspc::options::scoped_rifd());

    ::boost::program_options::variables_map vm
      ( gspc::testing::parse_command_line
          ( ::boost::unit_test::framework::master_test_suite().argc
          , ::boost::unit_test::framework::master_test_suite().argv
          , options_description
          )
      );

    gspc::util::temporary_path const shared_directory
      (gspc::testing::shared_directory (vm) / test_name);

    gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
      (shared_directory, vm);

    gspc::util::temporary_path const _installation_dir
      ( std::filesystem::path {shared_directory} / gspc::testing::unique_path()
      );
    auto const installation_dir {std::filesystem::path {_installation_dir}};

    gspc::set_application_search_path (vm, installation_dir);

    vm.notify();

    gspc::installation const installation (vm);

    gspc::testing::make_net_lib_install const make
      ( installation
      , network_name
      , gspc::testing::source_directory (vm)
      , installation_dir
      );

    gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                   , gspc::rifd::hostnames {vm}
                                   , gspc::rifd::port {vm}
                                   , installation
                                   );

    auto const num_workers
      { gspc::testing::random<unsigned int>{} (5, 1)
      };

    gspc::scoped_runtime_system const drts
      ( vm
      , installation
      , "work:" + std::to_string (num_workers)
      , rifds.entry_points()
      );

    std::multimap<std::string, gspc::pnet::type::value::value_type> const result
      ( gspc::client (drts).put_and_run
          ( gspc::workflow (make.pnet())
          , inputs
          )
      );

    GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
  }
}

// Test the simple shared example:
// - Creates a shared<string> that cleans up to a place named "cleanup"
// - Copies the shared to two separate places (refcount = 2)
// - Consumes both copies via module calls (refcount -> 0)
// - When the last reference is consumed, the original string value
//   automatically appears at the "cleanup" place as the workflow output
//
BOOST_AUTO_TEST_CASE (share_example_shared_simple)
{
  auto const test_value {gspc::testing::random<std::string>()()};

  run_shared_test
    ( "share_example_shared_simple"
    , "simple"
    , {{"value", test_value}}
    , {{"result", test_value}}
    );
}

// Test the full shared example with two independent shareds:
// - Creates two shared<string> values targeting different cleanup places
// - file_name shared: copied 3x, all consumed -> appears at "files" place
// - temp_name shared: copied 2x, all consumed -> appears at "temps" place
// - Demonstrates multiple shareds with different cleanup places coexisting
//
BOOST_AUTO_TEST_CASE (share_example_shared)
{
  auto random_value {gspc::testing::random<std::string>{}};
  auto const file_value {std::invoke (random_value)};
  auto const temp_value {std::invoke (random_value)};

  run_shared_test
    ( "share_example_shared"
    , "full"
    , { {"file_name", file_value}
      , {"temp_name", temp_value}
      }
    , { {"cleaned_file", file_value}
      , {"cleaned_temp", temp_value}
      }
    );
}

// Test creating a shared in a Petri net expression (not a module call):
// - Uses expression syntax: shared_cleanup (${value}) to create a shared
// - Copies the shared twice (refcount = 2)
// - Consumes both copies via module calls
// - When the last reference is consumed, the string appears at "cleanup" place
//
BOOST_AUTO_TEST_CASE (share_example_shared_expression)
{
  auto const value {std::invoke (gspc::testing::random<std::string>{})};

  run_shared_test
    ( "share_example_shared_expression"
    , "expression"
    , {{"value", value}}
    , {{"result", value}}
    );
}

// Test that passing a shared through transitions preserves refcount:
// - Creates a shared<string> (refcount = 1)
// - Passes through an expression with in/out ports (refcount unchanged)
// - Passes through a module call that returns the input (refcount unchanged)
// - Finally consumes it in a module call (refcount -> 0, cleanup triggered)
// - Verifies the original value appears at the cleanup place
//
BOOST_AUTO_TEST_CASE (share_example_shared_passthrough)
{
  auto const test_value
    { std::invoke (gspc::testing::random<std::string>{})
    };

  run_shared_test
    ( "share_example_shared_passthrough"
    , "passthrough"
    , {{"value", test_value}}
    , {{"result", test_value}}
    );
}

// Test consuming a shared in a pure expression (no module call):
// - Creates a shared<string> via module call
// - A transition takes the shared as input but has an empty expression body
// - The shared is consumed (discarded) by the expression, triggering cleanup
// - Verifies cleanup works even without explicit module-based consumption
//
BOOST_AUTO_TEST_CASE (share_example_shared_consume_in_expression)
{
  auto const test_value {gspc::testing::random<std::string>()()};

  run_shared_test
    ( "share_example_shared_consume_in_expression"
    , "consume_in_expression"
    , {{"value", test_value}}
    , {{"result", test_value}}
    );
}

// Test combining passthrough, copy, and consume operations:
// - Creates a shared<string> (refcount = 1)
// - Passes through an expression unchanged (refcount still 1)
// - Copies to two places via expression (refcount = 2)
// - Passes one copy through a module unchanged (refcount still 2)
// - Consumes both copies via module calls (refcount -> 0, cleanup triggered)
//
BOOST_AUTO_TEST_CASE (share_example_shared_mixed)
{
  auto const test_value {gspc::testing::random<std::string>()()};

  run_shared_test
    ( "share_example_shared_mixed"
    , "mixed"
    , {{"value", test_value}}
    , {{"result", test_value}}
    );
}

// Test that a cleanup place (shared_sink) can receive both cleanup tokens
// and regular output from transitions:
// - Creates a shared<string> from "shared_value" input, consumes it
// - Another transition puts "explicit_value" directly to the same place
// - Both values appear at the output, showing cleanup places are regular places
//
BOOST_AUTO_TEST_CASE (share_example_shared_sink_with_output)
{
  auto random_value {gspc::testing::random<std::string>{}};
  auto const shared_value {std::invoke (random_value)};
  auto const explicit_value {std::invoke (random_value)};

  run_shared_test
    ( "share_example_shared_sink_with_output"
    , "sink_with_output"
    , { {"shared_value", shared_value}
      , {"explicit_value", explicit_value}
      }
    , { {"result", shared_value}
      , {"result", explicit_value}
      }
    );
}

// Test that shared works with int values (not just strings):
// - Creates a shared<int> that cleans up to "cleanup" place
// - Copies twice, consumes both copies
// - When the last reference is consumed, the int value appears at cleanup
//
BOOST_AUTO_TEST_CASE (share_example_shared_int_type)
{
  auto const test_value {std::invoke (gspc::testing::random<int>{})};

  run_shared_test
    ( "share_example_shared_int_type"
    , "int_type"
    , {{"value", test_value}}
    , {{"result", test_value}}
    );
}

// Test that shared works with long values:
// - Creates a shared<long> that cleans up to "cleanup" place
// - Copies twice, consumes both copies
// - When the last reference is consumed, the long value appears at cleanup
//
BOOST_AUTO_TEST_CASE (share_example_shared_long_type)
{
  auto const test_value {std::invoke (gspc::testing::random<long>{})};

  run_shared_test
    ( "share_example_shared_long_type"
    , "long_type"
    , {{"value", test_value}}
    , {{"result", test_value}}
    );
}

// Test that shared works with list values:
// - Creates a shared<list> containing 5 random integers
// - Copies twice, consumes both copies
// - When the last reference is consumed, the entire list appears at cleanup
//
BOOST_AUTO_TEST_CASE (share_example_shared_list_type)
{
  auto random_value {gspc::testing::random<int>{}};
  std::list<gspc::pnet::type::value::value_type> const test_value
    { std::invoke (random_value)
    , std::invoke (random_value)
    , std::invoke (random_value)
    , std::invoke (random_value)
    , std::invoke (random_value)
    };

  run_shared_test
    ( "share_example_shared_list_type"
    , "list_type"
    , {{"value", test_value}}
    , {{"result", test_value}}
    );
}

// Test that shared works with user-defined struct types:
// - Defines a "point" struct with x and y int fields
// - Creates a shared<point> that cleans up to "cleanup" place
// - Copies twice, consumes both copies
// - When the last reference is consumed, the struct appears at cleanup
//
BOOST_AUTO_TEST_CASE (share_example_shared_struct_type)
{
  auto random_value {gspc::testing::random<int>{}};
  auto const x_value {std::invoke (random_value)};
  auto const y_value {std::invoke (random_value)};

  // Create expected struct (point with x and y fields)
  gspc::pnet::type::value::value_type expected_point;
  gspc::pnet::type::value::poke ("x", expected_point, x_value);
  gspc::pnet::type::value::poke ("y", expected_point, y_value);

  run_shared_test
    ( "share_example_shared_struct_type"
    , "struct_type"
    , { {"x", x_value}
      , {"y", y_value}
      }
    , {{"result", expected_point}}
    );
}

// Test that shareds nested inside a list are properly cleaned up:
// - Creates a pnet list containing one shared<int> element
// - Consumes the list in a module call
// - When the list is consumed, the nested shared's refcount drops to 0
// - The int value automatically appears at the cleanup place
//
BOOST_AUTO_TEST_CASE (share_example_shared_nested_in_list)
{
  auto const test_value {std::invoke (gspc::testing::random<int>{})};

  run_shared_test
    ( "share_example_shared_nested_in_list"
    , "nested_in_list"
    , {{"value", test_value}}
    , {{"result", test_value}}
    );
}

// Test that multiple shareds nested in a list are all cleaned up:
// - Creates a pnet list containing three shared<int> elements
// - Consumes the list in a module call
// - All three nested shareds trigger cleanup, producing three output values
//
BOOST_AUTO_TEST_CASE (share_example_shared_multiple_nested_in_list)
{
  auto random_value {gspc::testing::random<int>{}};
  auto const value1 {std::invoke (random_value)};
  auto const value2 {std::invoke (random_value)};
  auto const value3 {std::invoke (random_value)};

  run_shared_test
    ( "share_example_shared_multiple_nested_in_list"
    , "multiple_nested_in_list"
    , { {"value1", value1}
      , {"value2", value2}
      , {"value3", value3}
      }
    , { {"result", value1}
      , {"result", value2}
      , {"result", value3}
      }
    );
}

// Test that a shared nested in a struct field is properly cleaned up:
// - Creates a struct with a regular long field and a shared<int> field
// - Consumes the struct in a module call
// - The nested shared<int> field triggers cleanup, producing the int value
// - The regular field is discarded without cleanup
//
BOOST_AUTO_TEST_CASE (share_example_shared_nested_in_struct)
{
  auto const test_value {std::invoke (gspc::testing::random<int>{})};
  auto const other_value {std::invoke (gspc::testing::random<long>{})};

  run_shared_test
    ( "share_example_shared_nested_in_struct"
    , "nested_in_struct"
    , { {"value", test_value}
      , {"other", other_value}
      }
    , {{"result", test_value}}
    );
}

// Test that multiple shared fields in a struct are all cleaned up:
// - Creates a struct with two shared<int> fields
// - Consumes the struct in a module call
// - Both shared fields trigger cleanup, producing two int values at output
//
BOOST_AUTO_TEST_CASE (share_example_shared_multiple_nested_in_struct)
{
  auto random_value {gspc::testing::random<int>{}};
  auto const value1 {std::invoke (random_value)};
  auto const value2 {std::invoke (random_value)};

  run_shared_test
    ( "share_example_shared_multiple_nested_in_struct"
    , "multiple_nested_in_struct"
    , { {"value1", value1}
      , {"value2", value2}
      }
    , { {"result", value1}
      , {"result", value2}
      }
    );
}

// Test cleanup with a shared containing a struct that has a shared field:
// - Creates an outer shared<struct> where the struct has a shared<int> field
// - When the outer shared is consumed:
//   - The struct value (with inner shared still as shared type) goes to outer_cleanup
//   - The inner shared also triggers cleanup, putting the int on inner_cleanup
// - Both cleanup places receive their respective values
//
BOOST_AUTO_TEST_CASE (share_example_shared_nested_cleanup)
{
  auto const inner_value {std::invoke (gspc::testing::random<int>{})};
  auto const regular_value {std::invoke (gspc::testing::random<long>{})};

  // Create expected outer struct (what ends up on outer_cleanup)
  // The shared_field stays as a shared for type consistency with the struct definition
  gspc::pnet::type::value::value_type expected_outer;
  gspc::pnet::type::value::poke ("regular_field", expected_outer, regular_value);
  gspc::pnet::type::value::poke
    ( "shared_field"
    , expected_outer
    , gspc::we::type::shared {inner_value, "inner_cleanup"}
    );

  run_shared_test
    ( "share_example_shared_nested_cleanup"
    , "nested_cleanup"
    , { {"inner_value", inner_value}
      , {"regular_value", regular_value}
      }
    , { {"outer_result", expected_outer}
      , {"inner_result", inner_value}
      }
    );
}

// Test that an inner shared referenced by multiple outer shareds is cleaned
// up only when all outer shareds are consumed:
// - Creates two outer shared<struct> values, each containing the same inner shared<int>
// - The inner shared's refcount is 2 (referenced from each outer struct)
// - Each outer shared has its own refcount of 1
// - When both outers are consumed, the inner's refcount drops to 0 and cleans up
// - Result: 2 outer struct values + 1 inner int value
//
BOOST_AUTO_TEST_CASE (share_example_shared_multiple_references_to_inner)
{
  auto const inner_value {std::invoke (gspc::testing::random<int>{})};

  // Create expected outer structs (what ends up on outer_cleanup_1 and outer_cleanup_2)
  gspc::pnet::type::value::value_type expected_outer_1;
  gspc::pnet::type::value::poke ("id", expected_outer_1, 1L);
  gspc::pnet::type::value::poke
    ( "inner"
    , expected_outer_1
    , gspc::we::type::shared (inner_value, "inner_cleanup")
    );

  gspc::pnet::type::value::value_type expected_outer_2;
  gspc::pnet::type::value::poke ("id", expected_outer_2, 2L);
  gspc::pnet::type::value::poke
    ( "inner"
    , expected_outer_2
    , gspc::we::type::shared (inner_value, "inner_cleanup")
    );

  run_shared_test
    ( "share_example_shared_multiple_references_to_inner"
    , "multiple_references_to_inner"
    , { {"inner_value", inner_value}
      }
    , { {"outer_result_1", expected_outer_1}
      , {"outer_result_2", expected_outer_2}
      , {"inner_result", inner_value}  // Inner cleaned up only once (after both outers consumed)
      }
    );
}

// Test that replacing a shared field in a struct cleans up the original:
// - Create a struct with a shared<int> field containing original_value
// - Replace the shared field with a new shared containing replacement_value
// - The original shared is no longer referenced, so it cleans up immediately
// - Consume the struct, which cleans up the replacement shared
// - Result: both original and replacement values appear at the cleanup place
//
BOOST_AUTO_TEST_CASE (share_example_shared_replace_shared_in_struct)
{
  auto random_value {gspc::testing::random<int>{}};
  auto const original_value {std::invoke (random_value)};
  auto const replacement_value {std::invoke (random_value)};

  run_shared_test
    ( "share_example_shared_replace_shared_in_struct"
    , "replace_shared_in_struct"
    , { {"original_value", original_value}
      , {"replacement_value", replacement_value}
      }
    , { {"result", original_value}      // Original shared cleaned up when replaced
      , {"result", replacement_value}    // Replacement cleaned up when struct consumed
      }
    );
}

// Test that cleanup places work with connect-number-of-tokens:
// - Creates three independent shared<string> values, all targeting "cleanup" place
// - Consumes all three (each triggers cleanup, 3 tokens appear at cleanup)
// - A transition uses connect-number-of-tokens to detect when all 3 arrive
// - Once the condition is met, tokens are collected into the result
//
BOOST_AUTO_TEST_CASE (share_example_shared_cleanup_with_number_of_tokens)
{
  auto random_value {gspc::testing::random<std::string>{}};
  auto const value1 {std::invoke (random_value)};
  auto const value2 {std::invoke (random_value)};
  auto const value3 {std::invoke (random_value)};

  run_shared_test
    ( "share_example_shared_cleanup_with_number_of_tokens"
    , "cleanup_with_number_of_tokens"
    , { {"value1", value1}
      , {"value2", value2}
      , {"value3", value3}
      }
    , { {"result", value1}
      , {"result", value2}
      , {"result", value3}
      }
    );
}

// Test that a cleanup place can receive external tokens via put_token:
// - Cleanup place is marked with shared_sink="true" and put_token="true"
// - Creates two shared<string> values that clean up internally
// - The test client injects two additional tokens via gspc::client::put_token()
// - All four tokens (2 internal + 2 external) are collected into a list result
// - Demonstrates external resource cleanup notification
//
BOOST_AUTO_TEST_CASE (share_example_shared_cleanup_with_put_token)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  gspc::util::temporary_path const shared_directory
    (gspc::testing::shared_directory (vm) / "share_example_shared_cleanup_with_put_token");

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  gspc::util::temporary_path const _installation_dir
    ( std::filesystem::path {shared_directory} / gspc::testing::unique_path()
    );
  auto const installation_dir {std::filesystem::path {_installation_dir}};

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::testing::make_net_lib_install const make
    ( installation
    , "cleanup_with_put_token"
    , gspc::testing::source_directory (vm)
    , installation_dir
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  gspc::scoped_runtime_system const drts
    ( vm
    , installation
    , "work:1"
    , rifds.entry_points()
    );

  gspc::client client (drts);

  // Generate random values for internal shareds
  auto random_value {gspc::testing::random<std::string>{}};
  auto const internal_value_1 (std::invoke (random_value));
  auto const internal_value_2 (std::invoke (random_value));

  // Submit the workflow with input values
  auto const job
    ( client.submit
        ( gspc::workflow (make.pnet())
        , { {"value1", internal_value_1}
          , {"value2", internal_value_2}
          }
        )
    );

  // Inject external cleanup tokens via put_token
  std::string const external_value_1 {"external_value_1"};
  std::string const external_value_2 {"external_value_2"};
  client.put_token (job, "external_cleanup", external_value_1);
  client.put_token (job, "external_cleanup", external_value_2);

  // Signal that we're done injecting tokens
  client.put_token (job, "done_flag", gspc::we::type::literal::control{});

  // Wait for the workflow to complete
  client.wait (job);

  auto const result (client.extract_result_and_forget_job (job));

  // The result should contain all cleanup tokens:
  // - 2 internal: from shared cleanup (random values)
  // - 2 external: "external_value_1", "external_value_2" (from put_token)
  // All collected into a list
  BOOST_REQUIRE_EQUAL (result.size(), 1);
  BOOST_REQUIRE_EQUAL (result.count ("result"), 1);

  auto const& result_value (result.find ("result")->second);
  auto const& result_list
    (::boost::get<std::list<gspc::pnet::type::value::value_type>> (result_value));

  BOOST_REQUIRE_EQUAL (result_list.size(), 4);

  // Convert to set of strings for easier comparison
  std::set<std::string> collected_values;
  for (auto const& v : result_list)
  {
    collected_values.insert (::boost::get<std::string> (v));
  }

  std::set<std::string> const expected_values
    { internal_value_1
    , internal_value_2
    , external_value_1
    , external_value_2
    };

  BOOST_REQUIRE_EQUAL (collected_values, expected_values);
}
