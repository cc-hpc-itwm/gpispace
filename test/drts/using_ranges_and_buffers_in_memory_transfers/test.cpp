// Copyright (C) 2020-2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <test/drts/using_ranges_and_buffers_in_memory_transfers/net_description.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>
#include <gspc/drts/virtual_memory.hpp>

#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/source_directory.hpp>
#include <gspc/testing/virtual_memory_socket_name_for_localhost.hpp>

#include <gspc/testing/printer/we/type/value.hpp>

#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/require_exception.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>

#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <regex>

#define MAX_ALIGNMENT 16
#define MAX_BUFFER_SIZE 100

#define GSPC_TESTING_REQUIRE_EXCEPTION_MATCHING_REGEX( TYPE               \
                                                         , REGEX              \
                                                         , REGEX_FORMAT       \
                                                         , WHEN...            \
                                                         )                    \
  do                                                                          \
  {                                                                           \
    std::string const expected_string (REGEX);                                \
    gspc::testing::require_exception                                     \
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
  ::boost::program_options::options_description options_description;       \
                                                                         \
  options_description.add (gspc::testing::options::shared_directory());           \
  options_description.add (gspc::testing::options::source_directory());           \
  options_description.add (gspc::options::installation());               \
  options_description.add (gspc::options::drts());                       \
  options_description.add (gspc::options::scoped_rifd());                \
  options_description.add (gspc::options::virtual_memory());             \
                                                                         \
  ::boost::program_options::variables_map vm                               \
    ( gspc::testing::parse_command_line                                           \
        ( ::boost::unit_test::framework::master_test_suite().argc          \
        , ::boost::unit_test::framework::master_test_suite().argv          \
        , options_description                                            \
        )                                                                \
    );                                                                   \
                                                                         \
  auto const test_case_dir                                               \
    ( fmt::format ("{}-{}-{}-{}"                                         \
                  , std::string (#TESTCASE)                              \
                  , BUFFSIZE                                             \
                  , OFFSET                                               \
                  , SIZE                                                 \
                  )                                                      \
    );                                                                   \
                                                                         \
  gspc::util::temporary_path const shared_directory                       \
    (gspc::testing::shared_directory (vm) / test_case_dir);                       \
                                                                         \
  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment \
    (shared_directory, vm);                                              \
                                                                         \
  gspc::util::temporary_path const _installation_dir                      \
    ( std::filesystem::path {shared_directory}                           \
    / gspc::testing::unique_path()                                           \
    );                                                                   \
  auto const installation_dir {std::filesystem::path {_installation_dir}};    \
                                                                         \
  gspc::set_application_search_path (vm, installation_dir);              \
  gspc::testing::set_virtual_memory_socket_name_for_localhost (vm);               \
                                                                         \
  vm.notify();                                                           \
                                                                         \
  gspc::installation const installation (vm);                            \
                                                                         \
  gspc::util::temporary_path const _workflow_dir                          \
    ( std::filesystem::path {shared_directory}                           \
    / gspc::testing::unique_path()                                           \
    );                                                                   \
  auto const workflow_dir {std::filesystem::path {_workflow_dir}};            \
                                                                         \
  std::ofstream                                                          \
    (workflow_dir / (std::string (#TESTCASE) + ".xpnet"))                \
      << drts::test::net_description                                     \
           ( gspc::testing::random<bool>{}() ? "get" : "put"        \
           , ( ALLOW_EMPTY_RANGES                                        \
             ? std::make_optional (true)                               \
             : ( gspc::testing::random<bool>{}()                    \
               ? std::make_optional (false)                            \
               : std::nullopt                                             \
               )                                                         \
             )                                                           \
           , WITH_ALIGNMENT                                              \
           );                                                            \
                                                                         \
  gspc::testing::make_net_lib_install const make                                  \
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
                 , gspc::testing::random<std::size_t>{}             \
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
      { gspc::testing::random<std::size_t>{} (MAX_BUFFER_SIZE, 1)
      , gspc::testing::random<std::size_t>{} (MAX_BUFFER_SIZE, 1)
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
  , ::boost::unit_test::data::make (std::vector<Sample> (empty_range_samples))
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

  decltype (result) const expected {{"done", gspc::we::type::literal::control()}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}

BOOST_DATA_TEST_CASE
  ( empty_ranges_are_not_allowed_if_the_flag_is_not_set_or_set_on_false
  , ::boost::unit_test::data::make (std::vector<Sample> (empty_range_samples))
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

  GSPC_TESTING_REQUIRE_EXCEPTION_MATCHING_REGEX
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
  , ::boost::unit_test::data::make (std::vector<Sample> (failing_samples))
  , sample
  )
{
  START_RUNTIME_CREATE_NET_AND_ALLOCATE_GLOBAL_MEMORY
    ( using_ranges_that_exceed_the_buffer_limits_is_not_allowed
    , sample.buffer_size
    , sample.offset
    , sample.size
    , gspc::testing::random<int>{} (1, 0)
    , gspc::testing::random<int>{} (1, 0)
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

  GSPC_TESTING_REQUIRE_EXCEPTION_MATCHING_REGEX
    ( std::runtime_error
    , "Job " + job_id + ": failed: error-message := "
      "worker-.*-1: call to 'test_module::task' failed: "
      "local range to large"
    , std::regex::basic
    , client.wait (job_id)
    );
}
