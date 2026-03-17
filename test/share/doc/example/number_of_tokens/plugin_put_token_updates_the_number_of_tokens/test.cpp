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
#include <gspc/we/type/value/peek.hpp>

#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/random/integral.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>

#include <boost/program_options.hpp>

#include <filesystem>

#include <algorithm>
#include <iterator>
#include <map>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <vector>

BOOST_AUTO_TEST_CASE
  ( number_of_tokens_plugin_put_token_updates_the_number_of_tokens
  )
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add_options()
    ( "plugin-path"
    , ::boost::program_options::value<std::string>()->required()
    , "the location of the Plugin"
    );

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  auto const plugin_path {vm.at ("plugin-path").as<std::string>()};

  gspc::util::temporary_path const shared_directory
    ( gspc::testing::shared_directory (vm)
    / "share_example_number_of_tokens_plugin_put_token_updates_the_number_of_tokens"
    );

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::testing::make_net const make
    ( installation
    , "plugin_put_token_updates_the_number_of_tokens"
    , gspc::testing::source_directory (vm)
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  gspc::scoped_runtime_system const drts { vm
                                         , installation
                                         , ""
                                         , rifds.entry_points()
                                         };

  auto const n {gspc::testing::random<unsigned long>{} (1000UL, 1UL)};
  auto const m {gspc::testing::random<unsigned long>{} (2 * n, n)};

  auto const result
    { gspc::client {drts}.put_and_run ( make.pnet()
                                      , { {"n", n}
                                        , {"m", m}
                                        , {"plugin_path", plugin_path}
                                        }
                                      )
    };

  decltype (result) const expected {{"sum", m * (m - 1UL) / 2UL}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
