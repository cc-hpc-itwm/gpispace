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

#include <drts/test/using_ranges_and_buffers_in_memory_transfers/net_description.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>
#include <drts/virtual_memory.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>
#include <test/virtual_memory_socket_name_for_localhost.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>

#include <iostream>
#include <regex>

#define MAX_ALIGNMENT 16
#define MAX_BUFFER_SIZE 100

#define FHG_UTIL_TESTING_REQUIRE_EXCEPTION_MATCHING_REGEX( TYPE               \
                                                         , REGEX              \
                                                         , REGEX_FORMAT       \
                                                         , WHEN...            \
                                                         )                    \
  do                                                                          \
  {                                                                           \
    std::string const expected_string (REGEX);                                \
    fhg::util::testing::require_exception                                     \
      ( [&]                                                                   \
        {                                                                     \
          WHEN;                                                               \
        }                                                                     \
      , TYPE (expected_string)                                                \
      , [&] (TYPE const&, TYPE const& catched)                                \
        {                                                                     \
          std::string const what (catched.what());                            \
          std::regex const expected_regex (expected_string, REGEX_FORMAT);    \
          if (!std::regex_match (what, expected_regex))                       \
          {                                                                   \
            throw std::logic_error ("'" + what + "' does not match regex");   \
          }                                                                   \
        }                                                                     \
      );                                                                      \
  }                                                                           \
  while (false)

#define START_RUNTIME_CREATE_NET_AND_ALLOCATE_GLOBAL_MEMORY( TESTCASE            \
                                                           , BUFFSIZE            \
                                                           , OFFSET              \
                                                           , SIZE                \
                                                           , ALLOW_EMPTY_RANGES  \
                                                           , WITH_ALIGNMENT      \
                                                           )                     \
  boost::program_options::options_description options_description;       \
                                                                         \
  options_description.add (test::options::shared_directory());           \
  options_description.add (test::options::source_directory());           \
  options_description.add (gspc::options::installation());               \
  options_description.add (gspc::options::drts());                       \
  options_description.add (gspc::options::scoped_rifd());                \
  options_description.add (gspc::options::virtual_memory());             \
                                                                         \
  boost::program_options::variables_map vm                               \
    ( test::parse_command_line                                           \
        ( boost::unit_test::framework::master_test_suite().argc          \
        , boost::unit_test::framework::master_test_suite().argv          \
        , options_description                                            \
        )                                                                \
    );                                                                   \
                                                                         \
  auto const test_case_dir                                               \
    (str ( boost::format ("%1%-%2%-%3%-%4%")                             \
         % std::string (#TESTCASE)                                       \
         % BUFFSIZE                                                      \
         % OFFSET                                                        \
         % SIZE                                                          \
         )                                                               \
    );                                                                   \
                                                                         \
  fhg::util::temporary_path const shared_directory                       \
    (test::shared_directory (vm) / test_case_dir);                       \
                                                                         \
  test::scoped_nodefile_from_environment const nodefile_from_environment \
    (shared_directory, vm);                                              \
                                                                         \
  fhg::util::temporary_path const _installation_dir                      \
    (shared_directory / boost::filesystem::unique_path());               \
  boost::filesystem::path const installation_dir (_installation_dir);    \
                                                                         \
  gspc::set_application_search_path (vm, installation_dir);              \
  test::set_virtual_memory_socket_name_for_localhost (vm);               \
                                                                         \
  vm.notify();                                                           \
                                                                         \
  gspc::installation const installation (vm);                            \
                                                                         \
  fhg::util::temporary_path const _workflow_dir                          \
    (shared_directory / boost::filesystem::unique_path());               \
  boost::filesystem::path const workflow_dir (_workflow_dir);            \
                                                                         \
  boost::filesystem::ofstream                                            \
    (workflow_dir / (std::string (#TESTCASE) + ".xpnet"))                \
      << drts::test::net_description                                     \
           ( fhg::util::testing::random<bool>{}() ? "get" : "put"        \
           , ( ALLOW_EMPTY_RANGES                                        \
             ? boost::make_optional (true)                               \
             : ( fhg::util::testing::random<bool>{}()                    \
               ? boost::make_optional (false)                            \
               : boost::none                                             \
               )                                                         \
             )                                                           \
           , WITH_ALIGNMENT                                              \
           );                                                            \
                                                                         \
  test::make_net_lib_install const make                                  \
    ( installation                                                       \
    , std::string (#TESTCASE)                                            \
    , workflow_dir                                                       \
    , installation_dir                                                   \
    );                                                                   \
                                                                         \
  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}             \
                                 , gspc::rifd::hostnames {vm}            \
                                 , gspc::rifd::port {vm}                 \
                                 , installation                          \
                                 );                                      \
                                                                         \
  gspc::scoped_runtime_system const drts                                 \
    ( vm                                                                 \
    , installation                                                       \
    , "worker:1," + std::to_string (MAX_ALIGNMENT + MAX_BUFFER_SIZE)     \
    , rifds.entry_points()                                               \
    );                                                                   \
                                                                         \
  gspc::vmem_allocation const allocation                                 \
    ( drts.alloc ( gspc::vmem::gaspi_segment_description()               \
                 , fhg::util::testing::random<std::size_t>{}             \
                     ( sample.buffer_size + 100                          \
                     /* 0-sized buffers are allowed, not allocations */  \
                     , std::max (std::size_t (1), sample.buffer_size)    \
                     )                                                   \
                 , "data"                                                \
                 )                                                       \
    )


namespace
{
  struct Sample
  {
    std::size_t buffer_size;
    std::size_t offset;
    std::size_t size;
  };

  std::ostream& operator<< (std::ostream& os, Sample const& sample)
  {
    return os << "{"
              << sample.buffer_size << ", "
              << sample.offset << ", "
              << sample.size
              << "}";
  }

  std::vector<Sample> const empty_range_samples
    { {0, 0, 0}
    , {0, 1, 0}
    , {1, 0, 0}
    , {1, 1, 0},
      { fhg::util::testing::random<std::size_t>{} (MAX_BUFFER_SIZE, 1)
      , fhg::util::testing::random<std::size_t>{} (MAX_BUFFER_SIZE, 1)
      , 0
      }
    };

  std::vector<Sample> const failing_samples
    { {0, 0, 1}
    , {0, 1, 1}
    , {1, 0, 2}
    , {1, 1, 1}
    , {1, 2, 1}
    };

  enum {EMPTY_RANGES_NOT_ALLOWED, EMPTY_RANGES_ALLOWED};
  enum {NO_ALIGNMENT_SPECIFIED, RANDOM_ALIGNMENT};
}

BOOST_DATA_TEST_CASE
  ( empty_ranges_are_allowed_if_the_flag_is_set_on_true
  , boost::unit_test::data::make (std::vector<Sample> (empty_range_samples))
  , sample
  )
{
  START_RUNTIME_CREATE_NET_AND_ALLOCATE_GLOBAL_MEMORY
    ( empty_ranges_are_allowed_if_the_flag_is_set_on_true
    , sample.buffer_size
    , sample.offset
    , sample.size
    , EMPTY_RANGES_ALLOWED
    , RANDOM_ALIGNMENT
    );

  auto const result
    ( gspc::client (drts).put_and_run
      ( gspc::workflow (make.pnet())
      , { {"global", allocation.global_memory_range()}
        , {"range_size", sample.size}
        , {"offset", sample.offset}
        , {"buffer_size", sample.buffer_size}
        }
      )
    );

  decltype (result) const expected {{"done", we::type::literal::control()}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}

BOOST_DATA_TEST_CASE
  ( empty_ranges_are_not_allowed_if_the_flag_is_not_set_or_set_on_false
  , boost::unit_test::data::make (std::vector<Sample> (empty_range_samples))
  , sample
  )
{
  START_RUNTIME_CREATE_NET_AND_ALLOCATE_GLOBAL_MEMORY
    ( empty_ranges_are_not_allowed_if_the_flag_is_not_set_or_set_on_false
    , sample.buffer_size
    , sample.offset
    , sample.size
    , EMPTY_RANGES_NOT_ALLOWED
    , RANDOM_ALIGNMENT
    );

  gspc::client client (drts);
  auto const job_id
    ( client.submit
        ( make.pnet()
        , { {"global", allocation.global_memory_range()}
          , {"range_size", sample.size}
          , {"offset", sample.offset}
          , {"buffer_size", sample.buffer_size}
          }
        )
    );

  FHG_UTIL_TESTING_REQUIRE_EXCEPTION_MATCHING_REGEX
    ( std::runtime_error
    , "Job " + job_id + ": failed: error-message := .*"
      "Attempting to transfer empty ranges! If this behavior is "
      "wanted, then please set the attribute \"allow-empty-ranges\" "
      "for memory transfers on true in the Petri net!"
    , std::regex::basic
    , client.wait (job_id)
    );
}

BOOST_DATA_TEST_CASE
  ( using_ranges_that_exceed_the_buffer_limits_is_not_allowed
  , boost::unit_test::data::make (std::vector<Sample> (failing_samples))
  , sample
  )
{
  START_RUNTIME_CREATE_NET_AND_ALLOCATE_GLOBAL_MEMORY
    ( using_ranges_that_exceed_the_buffer_limits_is_not_allowed
    , sample.buffer_size
    , sample.offset
    , sample.size
    , fhg::util::testing::random<int>{} (1, 0)
    , fhg::util::testing::random<int>{} (1, 0)
    );

  gspc::client client (drts);
  auto const job_id
    ( client.submit
        ( make.pnet()
        , { {"global", allocation.global_memory_range()}
          , {"range_size", sample.size}
          , {"offset", sample.offset}
          , {"buffer_size", sample.buffer_size}
          }
       )
    );

  FHG_UTIL_TESTING_REQUIRE_EXCEPTION_MATCHING_REGEX
    ( std::runtime_error
    , "Job " + job_id + ": failed: error-message := "
      "worker-.*-1: call to 'test_module::task' failed: "
      "local range to large"
    , std::regex::basic
    , client.wait (job_id)
    );
}
