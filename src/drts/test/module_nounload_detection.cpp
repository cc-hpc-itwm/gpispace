// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>

#include <we/loader/exceptions.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/test/framework.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include <map>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

//! Require an exception of type \a type_to be thrown in the snippet
//! \a when_, where the expected \c .what() is regex-matching \a
//! regex_ using the format \a regex_fmt.
//! \todo Move to util-generic.
#define FHG_UTIL_TESTING_REQUIRE_EXCEPTION_MATCHING_REGEX(type_, regex_, regex_fmt_, when_...) \
  do                                                                          \
  {                                                                           \
    std::string const expected_string (regex_);                               \
    fhg::util::testing::require_exception                                     \
      ( [&]                                                                   \
        {                                                                     \
          when_;                                                              \
        }                                                                     \
      , type_ (expected_string)                                               \
      , [&] (type_ const&, type_ const& catched)                              \
        {                                                                     \
          std::string const what (catched.what());                            \
          std::regex const expected_regex (expected_string, regex_fmt_);      \
          if (!std::regex_match (what, expected_regex))                       \
          {                                                                   \
            throw std::logic_error ("'" + what + "' does not match regex");   \
          }                                                                   \
        }                                                                     \
      );                                                                      \
  }                                                                           \
  while (false)

//! - command line parsing `vm`
//! - `shared_directory` + `lib_install_directory` + nodefile
//! - rifds + 1 worker "worker" per node
//! - `client`
#define COMMAND_LINE_PARSING_AND_SINGLE_WORKER_DRTS_SETUP                     \
  boost::program_options::variables_map vm                                    \
    ( test::parse_command_line                                                \
        ( boost::unit_test::framework::master_test_suite().argc               \
        , boost::unit_test::framework::master_test_suite().argv               \
        , boost::program_options::options_description()                       \
        . add (test::options::source_directory())                             \
        . add (test::options::shared_directory())                             \
        . add (gspc::options::installation())                                 \
        . add (gspc::options::drts())                                         \
        . add (gspc::options::scoped_rifd())                                  \
        )                                                                     \
    );                                                                        \
                                                                              \
  fhg::util::temporary_path const shared_directory                            \
    (test::shared_directory (vm) / boost::filesystem::unique_path());         \
  fhg::util::temporary_path const lib_install_directory                       \
    (shared_directory / boost::filesystem::unique_path());                    \
                                                                              \
  test::scoped_nodefile_from_environment const nodefile                       \
    (shared_directory, vm);                                                   \
  gspc::set_application_search_path (vm, lib_install_directory);              \
                                                                              \
  vm.notify();                                                                \
                                                                              \
  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}                  \
                                 , gspc::rifd::hostnames {vm}                 \
                                 , gspc::rifd::port {vm}                      \
                                 , vm                                         \
                                 );                                           \
  gspc::scoped_runtime_system const drts                                      \
    (vm, vm, "worker:1x1", rifds.entry_points());                             \
  gspc::client client (drts)


BOOST_AUTO_TEST_CASE (module_that_doesnt_unload)
{
  COMMAND_LINE_PARSING_AND_SINGLE_WORKER_DRTS_SETUP;

  test::make_net_lib_install const make
    ( vm
    , "module_with_nodelete_flag"
    , test::source_directory (vm) / "module_nounload_detection"
    , lib_install_directory
    );
  auto const library
    ((boost::filesystem::path (lib_install_directory) / "libm.so").string());

  auto const job_id (client.submit (make.pnet(), {}));

  FHG_UTIL_TESTING_REQUIRE_EXCEPTION_MATCHING_REGEX
    ( std::runtime_error
    , "Job " + job_id + ": failed: error-message := "
      "worker-.*-1: call to \'m::f\' failed: "
    + we::loader::module_load_failed
        ( library
        , we::loader::module_does_not_unload (library, {library}).what()
        ).what()
    , std::regex::basic
    , client.wait_and_extract (job_id)
    );
}

BOOST_AUTO_TEST_CASE (module_that_doesnt_unload_can_be_allowed)
{
  COMMAND_LINE_PARSING_AND_SINGLE_WORKER_DRTS_SETUP;

  test::make_net_lib_install const make
    ( vm
    , "module_with_nodelete_flag_and_allowed_to_have_rest"
    , test::source_directory (vm) / "module_nounload_detection"
    , lib_install_directory
    );

  auto const result (client.put_and_run (make.pnet(), {}));

  BOOST_REQUIRE_EQUAL (result.size(), 1);
  BOOST_REQUIRE_EQUAL (result.count ("done"), 1);
}

BOOST_AUTO_TEST_CASE (module_that_loads_library_that_doesnt_unload)
{
  COMMAND_LINE_PARSING_AND_SINGLE_WORKER_DRTS_SETUP;

  test::make_net_lib_install const make
    ( vm
    , "module_that_loads_library_that_doesnt_unload"
    , test::source_directory (vm) / "module_nounload_detection"
    , lib_install_directory
    );

  boost::filesystem::path const to_load (LIBRARY_TO_LOAD);

  auto const job_id (client.submit (make.pnet(), {{"path", to_load.string()}}));

  FHG_UTIL_TESTING_REQUIRE_EXCEPTION_MATCHING_REGEX
    ( std::runtime_error
    , "Job " + job_id + ": failed: error-message := "
      "worker-.*-1: call to \'m::f\' failed: "
    + we::loader::function_does_not_unload ("m", "f", {to_load}).what()
    , std::regex::basic
    , client.wait_and_extract (job_id)
    );
}

BOOST_AUTO_TEST_CASE (module_that_loads_library_that_doesnt_unload_allowed)
{
  COMMAND_LINE_PARSING_AND_SINGLE_WORKER_DRTS_SETUP;

  test::make_net_lib_install const make
    ( vm
    , "module_that_loads_library_that_doesnt_unload_and_allowed_to_have_rest"
    , test::source_directory (vm) / "module_nounload_detection"
    , lib_install_directory
    );

  boost::filesystem::path const to_load (LIBRARY_TO_LOAD);

  auto const result
    (client.put_and_run (make.pnet(), {{"path", to_load.string()}}));

  BOOST_REQUIRE_EQUAL (result.size(), 1);
  BOOST_REQUIRE_EQUAL (result.count ("done"), 1);
}

BOOST_AUTO_TEST_CASE (worker_state_via_static_still_possible)
{
  COMMAND_LINE_PARSING_AND_SINGLE_WORKER_DRTS_SETUP;

  test::make_net_lib_install const make
    ( vm
    , "module_with_static_counter"
    , test::source_directory (vm) / "module_nounload_detection"
    , lib_install_directory
    );

  auto const result (client.put_and_run (make.pnet(), {}));

  decltype (result) const expected
    { {"previous_invocations", 0}
    , {"previous_invocations", 1}
    , {"previous_invocations", 2}
    };

  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}
