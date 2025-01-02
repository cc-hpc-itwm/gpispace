// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>
#include <drts/virtual_memory.hpp>

#include <testing/make.hpp>
#include <testing/parse_command_line.hpp>
#include <testing/scoped_nodefile_from_environment.hpp>
#include <testing/shared_directory.hpp>
#include <testing/source_directory.hpp>
#include <testing/virtual_memory_socket_name_for_localhost.hpp>

#include <we/type/literal/control.hpp>
#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/boost/program_options/validators/positive_integral.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <fmt/core.h>
#include <algorithm>
#include <iterator>
#include <string>
#include <unordered_set>

BOOST_AUTO_TEST_CASE (share_example_subnetwork)
{
  constexpr char const* const option_block_size ("block-size");
  constexpr char const* const option_number_of_blocks ("number-of-blocks");
  constexpr char const* const option_memory_total ("memory-total");

  ::boost::program_options::options_description options_description;

  using positive_integral =
    fhg::util::boost::program_options::positive_integral<unsigned long>;

  options_description.add_options()
    ( option_block_size
    , ::boost::program_options::value<positive_integral>()->required()
    , "size of a single block in bytes"
    )
    ( option_number_of_blocks
    , ::boost::program_options::value<positive_integral>()->required()
    , "number of blocks to process"
    )
    ( option_memory_total
    , ::boost::program_options::value<positive_integral>()->required()
    , "virtual memory total"
    )
    ;

  options_description.add (test::options::shared_directory());
  options_description.add (test::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (gspc::options::virtual_memory());

  ::boost::program_options::variables_map vm
    ( test::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "share_example_subnetwork");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / ::boost::filesystem::unique_path());
  ::boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);
  test::set_virtual_memory_socket_name_for_localhost (vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net_lib_install const make
    ( installation
    , "process_all_blocks"
    , test::source_directory (vm)
    , installation_dir
    );

  unsigned long const block_size
    (vm.at (option_block_size).as<positive_integral>());
  unsigned long const number_of_blocks
    (vm.at (option_number_of_blocks).as<positive_integral>());
  unsigned long const memory_total
    (vm.at (option_memory_total).as<positive_integral>());

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    ( vm
    , installation
    , fmt::format
      ( "produce:2,{} process:1,{}"
       , 1 * block_size
       , 2 * block_size
       )
    , rifds.entry_points()
    );

  gspc::vmem_allocation const allocation_data
    ( drts.alloc ( gspc::vmem::gaspi_segment_description()
                 , memory_total
                 , "blocks"
                 )
    );

  auto const result
    ( gspc::client (drts).put_and_run
      ( gspc::workflow (make.pnet())
      , { {"memory", allocation_data.global_memory_range()}
        , {"block_size", block_size}
        , {"number_of_blocks", number_of_blocks}
        }
      )
    );

  std::multimap<std::string, pnet::type::value::value_type> expected;
  for (unsigned long block (0); block < number_of_blocks; ++block)
  {
    expected.emplace ("done", block);
  }
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
