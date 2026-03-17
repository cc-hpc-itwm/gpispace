// Copyright (C) 2020-2021,2023,2026 Fraunhofer ITWM
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
#include <gspc/testing/require_container_is_permutation.hpp>

#include <boost/program_options.hpp>

#include <map>
#include <string>

#include <gspc/util/print_container.hpp>

BOOST_AUTO_TEST_CASE (share_example_priorities)
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
    ( gspc::testing::shared_directory (vm)
    / "share_example_accumuate_values_from_multiple_submissions"
    );

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::testing::make_net const negate
    ( installation
    , "negate"
    , gspc::testing::source_directory (vm)
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "", rifds.entry_points());
  gspc::client client (drts);

  auto const random_long (gspc::testing::random<long>{});
  auto const workflow (negate.pnet());

  // the output of the second submission of the same workflow does
  // _not_ include the output of the first submission
  {
    auto const value (random_long());
    auto const result (client.put_and_run (workflow, {{"in", value}}));

    decltype (result) const expected {{"out", -value}};
    GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
  }
  {
    auto const value (random_long());
    auto const result (client.put_and_run (workflow, {{"in", value}}));

    decltype (result) const expected {{"out", -value}};
    GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
  }
}
