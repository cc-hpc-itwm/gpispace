// Copyright (C) 2014-2015,2020-2024,2026 Fraunhofer ITWM
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
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>

#include <map>

namespace
{
  std::multimap<std::string, gspc::pnet::type::value::value_type>
    get_result (std::string const& main, long n)
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
      (gspc::testing::shared_directory (vm) / main);

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

    gspc::testing::make_net const make
      ( installation
      , main
      , gspc::testing::source_directory (vm)
      );

    gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
    gspc::scoped_runtime_system const drts
      (vm, installation, "work:4", rifds.entry_points());

    return gspc::client (drts).put_and_run
      (gspc::workflow (make.pnet()), {{"n", n}});
  }
}

BOOST_AUTO_TEST_CASE (share_example_use_sequence)
{
  long const n (10);

  std::multimap<std::string, gspc::pnet::type::value::value_type> const result
    (get_result ("use_sequence", n));

  std::multimap<std::string, gspc::pnet::type::value::value_type> expected;
  for (long i (0); i < n; ++i)
  {
    for (long j (0); j < n - i - 1; ++j)
    {
      expected.emplace ("out", i);
    }
  }
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}

BOOST_AUTO_TEST_CASE (share_example_use_sequence_bounded)
{
  long const n (10);

  std::multimap<std::string, gspc::pnet::type::value::value_type> const result
    (get_result ("use_sequence_bounded", n));

  std::multimap<std::string, gspc::pnet::type::value::value_type> expected;
  for (long i (0); i < n; ++i)
  {
    expected.emplace ("out", i);
  }
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
