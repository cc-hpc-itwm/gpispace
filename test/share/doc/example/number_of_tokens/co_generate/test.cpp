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

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <fmt/core.h>
#include <algorithm>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

BOOST_AUTO_TEST_CASE (number_of_tokens_co_generator)
{
  ::boost::program_options::options_description options_description;

  options_description.add (test::options::shared_directory());
  options_description.add (test::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add_options()
    ( "net"
    , ::boost::program_options::value<std::string>()->required()
    , "the network source: one of {'expression', 'module'}"
    );

  ::boost::program_options::variables_map vm
    ( test::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  auto const net {vm.at ("net").as<std::string>()};

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / fmt::format ("share_example_number_of_tokens_co_generator-{}", net)
    );

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / ::boost::filesystem::unique_path());
  ::boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net_lib_install const make
    ( installation
    , net
    , test::source_directory (vm)
    , installation_dir
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  auto const number_of_process_workers
    {fhg::util::testing::random<unsigned long>{} (10UL, 1UL)};

  gspc::scoped_runtime_system const drts
    { vm
    , installation
    , fmt::format ("generate:1 process:{}", number_of_process_workers)
    , rifds.entry_points()
    };

  auto const n {fhg::util::testing::random<unsigned long>{} (1000UL, 1UL)};

  std::multimap<std::string, pnet::type::value::value_type> const result
    { gspc::client (drts)
      . put_and_run
         ( gspc::workflow (make.pnet())
         , { {"n", n}
           , {"number_of_process_workers", number_of_process_workers}
           }
         )
    };

  BOOST_REQUIRE_LE (result.size(), number_of_process_workers + 1UL);
                                // ^ task_ready                ^ sum

  auto const sumK
    { [] (auto k)
      {
        return k * (k - 1UL) / 2UL;
      }
    };

  auto total_sum {0UL};
  auto found_key_sum {false};

  for (auto const& [key, value] : result)
  {
    if (key == "sum")
    {
      BOOST_REQUIRE (!found_key_sum);

      found_key_sum = true;

      auto const sum {::boost::get<unsigned long> (value)};

      BOOST_REQUIRE_GE (sum, sumK (n));

      total_sum += sum;
    }
    else if (key == "task_ready")
    {
      auto const task {pnet::type::value::peek ("input", value)};

      BOOST_REQUIRE (task);

      total_sum += ::boost::get<unsigned long> (*task);
    }
    else
    {
      BOOST_FAIL
        ( "Unexpected result: "
          << key << " -> " << pnet::type::value::show (value)
        );
    }
  }

  BOOST_REQUIRE_EQUAL (total_sum, sumK (n + result.size() - 1UL));
}
