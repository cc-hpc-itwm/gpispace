// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
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

#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/random/integral.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>

#include <array>
#include <fmt/core.h>
#include <map>
#include <sstream>
#include <stdexcept>

BOOST_AUTO_TEST_CASE (number_of_tokens_out_many)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::testing::options::source_directory());
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
    ( gspc::testing::shared_directory (vm) / "share_example_number_of_tokens_out_many"
    );

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  gspc::util::temporary_path const _installation_dir
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  auto const installation_dir {std::filesystem::path {_installation_dir}};

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::testing::make_net_lib_install const make
    ( installation
    , "phi"
    , gspc::testing::source_directory (vm)
    , installation_dir
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  gspc::scoped_runtime_system const drts
    { vm
    , installation
    , "worker:1"
    , rifds.entry_points()
    };

  auto const phis
    { std::array<unsigned long, 99>
      {      1,  1,  2,  2,  4,  2,  6,  4,  6
      , 4,  10,  4, 12,  6,  8,  8, 16,  6, 18
      , 8,  12, 10, 22,  8, 20, 12, 18, 12, 28
      , 8,  30, 16, 20, 16, 24, 12, 36, 18, 24
      , 16, 40, 12, 42, 20, 24, 22, 46, 16, 42
      , 20, 32, 24, 52, 18, 40, 24, 36, 28, 58
      , 16, 60, 30, 36, 32, 48, 20, 66, 32, 44
      , 24, 70, 24, 72, 36, 40, 36, 60, 24, 78
      , 32, 54, 40, 82, 24, 64, 42, 56, 40, 88
      , 24, 72, 44, 60, 46, 72, 32, 96, 42, 60
      }
    };

  auto const n {gspc::testing::random<unsigned long>{} (99, 1)};

  std::multimap<std::string, gspc::pnet::type::value::value_type> const result
    { gspc::client (drts)
      . put_and_run (gspc::workflow (make.pnet()), {{"n", n}})
    };

  decltype (result) const expected {{"phi", phis.at (n - 1ul)}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
