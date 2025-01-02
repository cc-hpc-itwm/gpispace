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

#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <boost/program_options.hpp>

BOOST_AUTO_TEST_CASE (share_example_priorities)
{
  ::boost::program_options::options_description options_description;

  options_description.add (test::options::shared_directory());
  options_description.add (test::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());

  ::boost::program_options::variables_map vm
    ( test::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "share_example_priorities_sum");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net const sum
    ( installation
    , "sum"
    , test::source_directory (vm)
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "", rifds.entry_points());

  gspc::client client (drts);

  auto random_long (fhg::util::testing::random<long>{});
  ::we::type::literal::control t;

  // if a and b are provided, then c is the sum
  {
    auto const a (random_long());
    auto const b (random_long());

    auto const result
      (client.put_and_run (sum.pnet(), {{"a", a}, {"b", b}, {"trigger", t}}));

    decltype (result) const expected {{"c", a + b}};
    FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
  }

  // if only a or only b is provided, then c is this value
  {
    auto const a (random_long());

    auto const result
      (client.put_and_run (sum.pnet(), {{"a", a}, {"trigger", t}}));

    decltype (result) const expected {{"c", a}};
    FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
  }
  {
    auto const b (random_long());

    auto const result
      (client.put_and_run (sum.pnet(), {{"b", b}, {"trigger", t}}));

    decltype (result) const expected {{"c", b}};
    FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
  }

  // if no argument is provided, then the c is zero
  {
    auto const result (client.put_and_run (sum.pnet(), {{"trigger", t}}));

    decltype (result) const expected {{"c", 0L}};
    FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
  }
}
