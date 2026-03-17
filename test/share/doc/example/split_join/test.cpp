// Copyright (C) 2014-2016,2020-2026 Fraunhofer ITWM
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

#include <gspc/util/boost/program_options/validators/nonempty_string.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>

#include <algorithm>
#include <map>
#include <vector>

BOOST_AUTO_TEST_CASE (share_example_split_join)
{
  namespace validators = gspc::util::boost::program_options;

  ::boost::program_options::options_description options_description;

  constexpr char const* const option_main ("main");
  constexpr char const* const option_input ("input");
  constexpr char const* const option_expected_output ("expected-output");

  options_description.add_options()
    ( option_main
    , ::boost::program_options::value<validators::nonempty_string>()
      ->required()
    , "name of the main target"
    )
    ( option_input
    , ::boost::program_options::value<std::vector<long>>()->required()
    , "tokens to put"
    )
    ( option_expected_output
    , ::boost::program_options::value<std::vector<long>>()->required()
    , "output expected for the given input"
    )
    ;

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
    ( gspc::testing::shared_directory (vm)
    / gspc::testing::unique_path()
    );

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::testing::make_net const make
    ( installation
    , vm.at (option_main).as<validators::nonempty_string>()
    , gspc::testing::source_directory (vm)
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "work:4", rifds.entry_points());

  std::multimap<std::string, gspc::pnet::type::value::value_type> input;

  for (long i : vm.at (option_input).as<std::vector<long>>())
  {
    input.emplace ("I", i);
  }

  std::multimap<std::string, gspc::pnet::type::value::value_type> const result
    (gspc::client (drts).put_and_run (gspc::workflow (make.pnet()), input));

  std::multimap<std::string, gspc::pnet::type::value::value_type> expected;

  for (long i : vm.at (option_expected_output).as<std::vector<long>>())
  {
    expected.emplace ("O", i);
  }

  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
