// Copyright (C) 2021-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <gspc/we/type/value.hpp>
#include <gspc/testing/printer/we/type/value.hpp>
#include <gspc/we/type/value/poke.hpp>

#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/source_directory.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>

#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>

#include <vector>

namespace
{
  std::vector<unsigned long> elements()
  {
    return {1,10,100,1000,10000,100000,1000000};
  }
}

BOOST_DATA_TEST_CASE
  ( parallel_reduce_manual_expression
  , elements()
  , n
  )
{
  boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());

  boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  gspc::util::temporary_path const shared_directory
    (gspc::testing::shared_directory (vm) / "parallel_reduce_expression");

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::testing::make_net const make
    ( installation
    , "manual"
    , gspc::testing::source_directory (vm)
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  gspc::scoped_runtime_system const drts
    (vm, installation, "", rifds.entry_points());

  auto const result
    ( gspc::client (drts).put_and_run
        (gspc::workflow (make.pnet()), {{"n", n}})
    );

  gspc::pnet::type::value::value_type P;
  gspc::pnet::type::value::poke ("value", P, (n + 1ul) * n / 2ul);
  decltype (result) const expected {{"P", P}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
