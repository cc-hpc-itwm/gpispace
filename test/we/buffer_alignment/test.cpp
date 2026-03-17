// Copyright (C) 2020-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <test/we/buffer_alignment/net_description.hpp>
#include <test/we/buffer_alignment/nets_using_buffers.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>
#include <gspc/drts/virtual_memory.hpp>

#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/virtual_memory_socket_name_for_localhost.hpp>

#include <gspc/we/type/value.hpp>
#include <gspc/testing/printer/we/type/value.hpp>

#include <gspc/util/boost/program_options/validators/positive_integral.hpp>

#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/require_exception.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>

#include <fstream>

#define START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET_GEN(FILE,NET) \
  ::boost::program_options::options_description options_description;       \
  options_description.add (gspc::testing::options::shared_directory());           \
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
  gspc::util::temporary_path const shared_directory                       \
    ( gspc::testing::shared_directory (vm)                                        \
    / "buffer_alignment"                                                 \
    );                                                                   \
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
  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}             \
                                 , gspc::rifd::hostnames {vm}            \
                                 , gspc::rifd::port {vm}                 \
                                 , installation                          \
                                 );                                      \
                                                                         \
  gspc::util::temporary_path const _workflow_dir                          \
    ( std::filesystem::path {shared_directory}                           \
    / gspc::testing::unique_path()                                           \
    );                                                                   \
  auto const workflow_dir {std::filesystem::path {_workflow_dir}};            \
                                                                         \
  std::ofstream                                                          \
    (workflow_dir / (std::string (#FILE) + ".xpnet")) << NET;            \
                                                                         \
  gspc::testing::make_net_lib_install const make                                  \
    ( installation                                                       \
    , #FILE                                                              \
    , workflow_dir                                                       \
    , installation_dir                                                   \
    );                                                                   \
                                                                         \
  gspc::scoped_runtime_system const drts                                 \
    ( vm                                                                 \
    , installation                                                       \
    , "worker:1," + std::to_string (local_memory_size)                   \
    , rifds.entry_points()                                               \
    )

#define START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET(NET)          \
  unsigned long local_memory_size (0);                                   \
                                                                         \
  START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET_GEN                 \
    ( NET                                                                \
    , gspc::we::test::buffer_alignment::NET (local_memory_size)                \
    )                                                                    \

BOOST_AUTO_TEST_CASE (arbitrary_buffer_sizes_and_alignments)
{
  START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET
    (net_with_arbitrary_buffer_sizes_and_alignments);

  std::multimap<std::string, gspc::pnet::type::value::value_type> result;

  BOOST_REQUIRE_NO_THROW
    ( result = gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"start", gspc::we::type::literal::control()}})
    );

  decltype (result) const expected {{"done", gspc::we::type::literal::control()}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}

BOOST_AUTO_TEST_CASE (arbitrary_buffer_sizes_and_default_alignments)
{
  START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET
    (net_with_arbitrary_buffer_sizes_and_default_alignments);

  std::multimap<std::string, gspc::pnet::type::value::value_type> result;

  BOOST_REQUIRE_NO_THROW
    ( result = gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"start", gspc::we::type::literal::control()}})
    );

  decltype (result) const expected {{"done", gspc::we::type::literal::control()}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}

BOOST_AUTO_TEST_CASE (arbitrary_buffer_sizes_and_mixed_alignments)
{
  START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET
    (net_with_arbitrary_buffer_sizes_and_mixed_alignments);

  std::multimap<std::string, gspc::pnet::type::value::value_type> result;

  BOOST_REQUIRE_NO_THROW
    ( result = gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"start", gspc::we::type::literal::control()}})
    );

  decltype (result) const expected {{"done", gspc::we::type::literal::control()}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}

namespace
{
  std::vector<gspc::we::test::buffer_alignment::BufferInfo> documentation_example()
  {
    return { gspc::we::test::buffer_alignment::BufferInfo {"a", 1, 2}
           , gspc::we::test::buffer_alignment::BufferInfo {"b", 9, 4}
           };
  }
}

BOOST_AUTO_TEST_CASE (documentation_example_less_than_align_best_fails)
{
  unsigned long local_memory_size (10);

  START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET_GEN
    ( documentation_example_less_than_align_best_fails
    , gspc::we::test::buffer_alignment::create_net_description (documentation_example())
    );

  BOOST_REQUIRE_EXCEPTION
    ( gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"start", gspc::we::type::literal::control()}})
    , std::runtime_error
    , [](std::runtime_error const& exc)
      {
        return std::string (exc.what()).find
          ("Please take into account also the buffer alignments "
           "when allocating local shared memory!"
          ) != std::string::npos;
      }
    );
}

BOOST_AUTO_TEST_CASE (documentation_example_at_least_align_worst_works)
{
  unsigned long local_memory_size (14);

  START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET_GEN
    ( documentation_example_at_least_align_worst_works
    , gspc::we::test::buffer_alignment::create_net_description (documentation_example())
    );

  std::multimap<std::string, gspc::pnet::type::value::value_type> result;

  BOOST_REQUIRE_NO_THROW
    ( result = gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"start", gspc::we::type::literal::control()}})
    );

  decltype (result) const expected {{"done", gspc::we::type::literal::control()}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}
