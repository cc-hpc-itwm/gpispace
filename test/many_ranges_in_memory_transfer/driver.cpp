// Copyright (C) 2020-2021,2023-2024,2026 Fraunhofer ITWM
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
#include <gspc/testing/fmt_directory.hpp>
#include <gspc/testing/virtual_memory_socket_name_for_localhost.hpp>

#include <gspc/we/type/value.hpp>
#include <gspc/testing/printer/we/type/value.hpp>

#include <gspc/util/boost/program_options/validators/positive_integral.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>

#include <list>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE (many_ranges_in_memory_transfer)
{
  ::boost::program_options::options_description options_description;

  constexpr char const* const S ("size-of-range");
  constexpr char const* const D ("distance-between-ranges");
  constexpr char const* const N ("number-of-ranges");

  using PositiveUL =
    gspc::util::boost::program_options::positive_integral<unsigned long>;

  options_description.add_options()
    (S, ::boost::program_options::value<PositiveUL>()->required())
    (D, ::boost::program_options::value<unsigned long>()->required())
    (N, ::boost::program_options::value<PositiveUL>()->required())
    ;
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::fmt_directory());
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
    (gspc::testing::shared_directory (vm) / "many_ranges_in_memory_transfer");

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
    , "many_ranges_in_memory_transfer"
    , gspc::testing::source_directory (vm)
    , installation_dir
    , gspc::testing::option::options()
    . add<gspc::testing::option::gen::include> (std::filesystem::path {gspc::testing::fmt_directory (vm).string()} / "include")
    . add<gspc::testing::option::gen::cxx_flag> ("-DFMT_HEADER_ONLY")
    );

  unsigned long const size_of_range (vm.at (S).as<PositiveUL>());
  unsigned long const distance_between_ranges (vm.at (D).as<unsigned long>());
  unsigned long const number_of_ranges (vm.at (N).as<PositiveUL>());

  auto const data
    (gspc::testing::randoms<std::vector<char>>
      (number_of_ranges * (size_of_range + distance_between_ranges))
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  gspc::scoped_runtime_system const drts
    ( vm
    , installation
    , "worker:1," + std::to_string (number_of_ranges * size_of_range)
    , rifds.entry_points()
    );

  gspc::vmem_allocation const allocation
    ( drts.alloc_and_fill ( gspc::vmem::gaspi_segment_description()
                          , data.size()
                          , "data"
                          , data.data()
                          )
    );

  std::list<gspc::pnet::type::value::value_type> global;

  for (unsigned long range (0); range < number_of_ranges; ++range)
  {
    global.emplace_back
      ( allocation.global_memory_range
          ( range * (size_of_range + distance_between_ranges)
          , size_of_range
          )
      );
  }

  auto const result
    ( gspc::client (drts).put_and_run
      ( gspc::workflow (make.pnet())
      , { {"data", gspc::we::type::bytearray (data)}
        , {"global", global}
        , {"size_of_range", size_of_range}
        , {"distance_between_ranges", distance_between_ranges}
        , {"number_of_ranges", number_of_ranges}
        }
      )
    );

  decltype (result) const expected {{"out", gspc::we::type::literal::control()}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
