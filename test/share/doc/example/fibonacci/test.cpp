// Copyright (C) 2026 Fraunhofer ITWM
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
#include <gspc/we/type/value/poke.hpp>

#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>

#include <boost/program_options.hpp>

#include <filesystem>

#include <map>
#include <sstream>
#include <stdexcept>

BOOST_AUTO_TEST_CASE (share_example_fibonacci)
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
    (gspc::testing::shared_directory (vm) / "share_example_fibonacci");

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::testing::make_net const make
    ( installation
    , "fibonacci"
    , gspc::testing::source_directory (vm)
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  gspc::scoped_runtime_system const drts
    (vm, installation, "worker:1", rifds.entry_points());

  auto const result
    ( gspc::client (drts).put_and_run
      ( gspc::workflow (make.pnet())
      , { {"n", gspc::pnet::type::value::bigint_type (0)}
        , {"n", gspc::pnet::type::value::bigint_type (1)}
        , {"n", gspc::pnet::type::value::bigint_type (2)}
        , {"n", gspc::pnet::type::value::bigint_type (3)}
        , {"n", gspc::pnet::type::value::bigint_type (4)}
        , {"n", gspc::pnet::type::value::bigint_type (5)}
        , {"n", gspc::pnet::type::value::bigint_type (10)}
        , {"n", gspc::pnet::type::value::bigint_type (50)}
        , {"n", gspc::pnet::type::value::bigint_type (100)}
        , {"n", gspc::pnet::type::value::bigint_type (150)}
        }
      )
    );

  auto const fib
    { [] (auto n)
      {
        gspc::pnet::type::value::bigint_type x {0};
        gspc::pnet::type::value::bigint_type y {1};

        for (auto i {0}; i != n; ++i)
        {
          gspc::pnet::type::value::bigint_type z {x + y};
          x = y;
          y = z;
        }

        gspc::pnet::type::value::value_type v;
        gspc::pnet::type::value::poke ("n", v, gspc::pnet::type::value::bigint_type (n));
        gspc::pnet::type::value::poke ("value", v, x);

        return v;
      }
    };

  decltype (result) const expected
    { {"result", fib (0)}
    , {"result", fib (1)}
    , {"result", fib (2)}
    , {"result", fib (3)}
    , {"result", fib (4)}
    , {"result", fib (5)}
    , {"result", fib (10)}
    , {"result", fib (50)}
    , {"result", fib (100)}
    , {"result", fib (150)}
    };
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
