// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <testing/make.hpp>
#include <testing/parse_command_line.hpp>
#include <testing/scoped_nodefile_from_environment.hpp>
#include <testing/shared_directory.hpp>
#include <testing/source_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>
#include <we/type/value/peek.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/random/integral.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

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

  options_description.add (test::options::shared_directory());
  options_description.add (test::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add_options()
    ( "plugin-path"
    , ::boost::program_options::value<std::string>()->required()
    , "the location of the Plugin"
    );

  ::boost::program_options::variables_map vm
    ( test::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  auto const plugin_path {vm.at ("plugin-path").as<std::string>()};

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / "share_example_number_of_tokens_plugin_put_token_updates_the_number_of_tokens"
    );

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net const make
    ( installation
    , "plugin_put_token_updates_the_number_of_tokens"
    , test::source_directory (vm)
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

  auto const n {fhg::util::testing::random<unsigned long>{} (1000UL, 1UL)};
  auto const m {fhg::util::testing::random<unsigned long>{} (2 * n, n)};

  auto const result
    { gspc::client {drts}.put_and_run ( make.pnet()
                                      , { {"n", n}
                                        , {"m", m}
                                        , {"plugin_path", plugin_path}
                                        }
                                      )
    };

  decltype (result) const expected {{"sum", m * (m - 1UL) / 2UL}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
