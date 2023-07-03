// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <we/test/buffer_alignment/net_description.hpp>
#include <we/test/buffer_alignment/nets_using_buffers.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>
#include <drts/virtual_memory.hpp>

#include <testing/make.hpp>
#include <testing/parse_command_line.hpp>
#include <testing/scoped_nodefile_from_environment.hpp>
#include <testing/shared_directory.hpp>
#include <testing/virtual_memory_socket_name_for_localhost.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/boost/program_options/validators/positive_integral.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#define START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET_GEN(FILE,NET) \
  ::boost::program_options::options_description options_description;       \
  options_description.add (test::options::shared_directory());           \
  options_description.add (gspc::options::installation());               \
  options_description.add (gspc::options::drts());                       \
  options_description.add (gspc::options::scoped_rifd());                \
  options_description.add (gspc::options::virtual_memory());             \
                                                                         \
  ::boost::program_options::variables_map vm                               \
    ( test::parse_command_line                                           \
        ( ::boost::unit_test::framework::master_test_suite().argc          \
        , ::boost::unit_test::framework::master_test_suite().argv          \
        , options_description                                            \
        )                                                                \
    );                                                                   \
                                                                         \
  fhg::util::temporary_path const shared_directory                       \
    ( test::shared_directory (vm)                                        \
    / "buffer_alignment"                                                 \
    );                                                                   \
                                                                         \
  test::scoped_nodefile_from_environment const nodefile_from_environment \
    (shared_directory, vm);                                              \
                                                                         \
  fhg::util::temporary_path const _installation_dir                      \
    (shared_directory / ::boost::filesystem::unique_path());               \
  ::boost::filesystem::path const installation_dir (_installation_dir);    \
                                                                         \
  gspc::set_application_search_path (vm, installation_dir);              \
  test::set_virtual_memory_socket_name_for_localhost (vm);               \
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
  fhg::util::temporary_path const _workflow_dir                          \
    (shared_directory / ::boost::filesystem::unique_path());               \
  ::boost::filesystem::path const workflow_dir (_workflow_dir);            \
                                                                         \
  ::boost::filesystem::ofstream                                            \
    (workflow_dir / (std::string (#FILE) + ".xpnet")) << NET;            \
                                                                         \
  test::make_net_lib_install const make                                  \
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
    , we::test::buffer_alignment::NET (local_memory_size)                \
    )                                                                    \

BOOST_AUTO_TEST_CASE (arbitrary_buffer_sizes_and_alignments)
{
  START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET
    (net_with_arbitrary_buffer_sizes_and_alignments);

  std::multimap<std::string, pnet::type::value::value_type> result;

  BOOST_REQUIRE_NO_THROW
    ( result = gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"start", we::type::literal::control()}})
    );

  decltype (result) const expected {{"done", we::type::literal::control()}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}

BOOST_AUTO_TEST_CASE (arbitrary_buffer_sizes_and_default_alignments)
{
  START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET
    (net_with_arbitrary_buffer_sizes_and_default_alignments);

  std::multimap<std::string, pnet::type::value::value_type> result;

  BOOST_REQUIRE_NO_THROW
    ( result = gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"start", we::type::literal::control()}})
    );

  decltype (result) const expected {{"done", we::type::literal::control()}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}

BOOST_AUTO_TEST_CASE (arbitrary_buffer_sizes_and_mixed_alignments)
{
  START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET
    (net_with_arbitrary_buffer_sizes_and_mixed_alignments);

  std::multimap<std::string, pnet::type::value::value_type> result;

  BOOST_REQUIRE_NO_THROW
    ( result = gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"start", we::type::literal::control()}})
    );

  decltype (result) const expected {{"done", we::type::literal::control()}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}

namespace
{
  std::vector<we::test::buffer_alignment::BufferInfo> documentation_example()
  {
    return { we::test::buffer_alignment::BufferInfo {"a", 1, 2}
           , we::test::buffer_alignment::BufferInfo {"b", 9, 4}
           };
  }
}

BOOST_AUTO_TEST_CASE (documentation_example_less_than_align_best_fails)
{
  unsigned long local_memory_size (10);

  START_DRTS_WITH_SINGLE_WORKER_AND_CREATE_PETRI_NET_GEN
    ( documentation_example_less_than_align_best_fails
    , we::test::buffer_alignment::create_net_description (documentation_example())
    );

  BOOST_REQUIRE_EXCEPTION
    ( gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"start", we::type::literal::control()}})
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
    , we::test::buffer_alignment::create_net_description (documentation_example())
    );

  std::multimap<std::string, pnet::type::value::value_type> result;

  BOOST_REQUIRE_NO_THROW
    ( result = gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"start", we::type::literal::control()}})
    );

  decltype (result) const expected {{"done", we::type::literal::control()}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}
