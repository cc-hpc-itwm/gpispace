// Copyright (C) 2014-2015,2020-2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

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

#include <gspc/we/type/literal/control.hpp>
#include <gspc/we/type/value.hpp>
#include <gspc/testing/printer/we/type/value.hpp>

#include <gspc/util/boost/program_options/validators/positive_integral.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>

#include <map>

BOOST_AUTO_TEST_CASE (share_example_vmem_1_to_n)
{
  namespace validators = gspc::util::boost::program_options;

  ::boost::program_options::options_description options_description;

  constexpr char const* const option_num_bytes ("num-bytes");

  options_description.add_options()
    ( option_num_bytes
    , ::boost::program_options::value
      <validators::positive_integral<unsigned long>>()->required()
    , "size of bytes per worker"
    )
    ;

  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (gspc::options::virtual_memory());

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  gspc::util::temporary_path const shared_directory
    (gspc::testing::shared_directory (vm) / "share_example_vmem_1_to_n");

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  gspc::util::temporary_path const _installation_dir
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  auto const installation_dir {std::filesystem::path {_installation_dir}};

  gspc::set_application_search_path (vm, installation_dir);
  gspc::testing::set_virtual_memory_socket_name_for_localhost (vm);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::testing::make_net_lib_install const make
    ( installation
    , "vmem_1_to_n"
    , gspc::testing::source_directory (vm)
    , installation_dir
    );

  unsigned long const num_bytes
    (vm.at (option_num_bytes).as<validators::positive_integral<unsigned long>>());

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    ( vm
    , installation
    , "worker:1," + std::to_string (num_bytes)
    , rifds.entry_points()
    );

  gspc::vmem_allocation const allocation_data
    ( drts.alloc ( gspc::vmem::gaspi_segment_description()
                 , num_bytes
                 , "data"
                 )
    );

  std::multimap<std::string, gspc::pnet::type::value::value_type> const result
    ( gspc::client (drts).put_and_run
      ( gspc::workflow (make.pnet())
      , { {"memory", allocation_data.global_memory_range()}
        , {"outer", 5L}
        , {"inner", 5L}
        , {"seed", 3141L}
        }
      )
    );

  decltype (result) const expected {{"out", gspc::we::type::literal::control()}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
