// Copyright (C) 2023-2026 Fraunhofer ITWM
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
#include <gspc/testing/unique_path.hpp>

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

  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add_options()
    ( "net"
    , ::boost::program_options::value<std::string>()->required()
    , "the network source: one of {'expression', 'module'}"
    );

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  auto const net {vm.at ("net").as<std::string>()};

  gspc::util::temporary_path const shared_directory
    ( gspc::testing::shared_directory (vm)
    / fmt::format ("share_example_number_of_tokens_co_generator-{}", net)
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
    , net
    , gspc::testing::source_directory (vm)
    , installation_dir
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  auto const number_of_process_workers
    {gspc::testing::random<unsigned long>{} (10UL, 1UL)};

  gspc::scoped_runtime_system const drts
    { vm
    , installation
    , fmt::format ("generate:1 process:{}", number_of_process_workers)
    , rifds.entry_points()
    };

  auto const n {gspc::testing::random<unsigned long>{} (1000UL, 1UL)};

  std::multimap<std::string, gspc::pnet::type::value::value_type> const result
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
      auto const task {gspc::pnet::type::value::peek ("input", value)};

      BOOST_REQUIRE (task);

      total_sum += ::boost::get<unsigned long> (task->get());
    }
    else
    {
      BOOST_FAIL
        ( "Unexpected result: "
          << key << " -> " << gspc::pnet::type::value::show (value)
        );
    }
  }

  BOOST_REQUIRE_EQUAL (total_sum, sumK (n + result.size() - 1UL));
}
