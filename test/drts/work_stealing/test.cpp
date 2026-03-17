// Copyright (C) 2020-2023,2026 Fraunhofer ITWM
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

#include <gspc/util/first_then.hpp>
#include <gspc/util/read_lines.hpp>
#include <gspc/util/temporary_file.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/unique_path.hpp>

#include <gspc/testing/printer/we/type/value.hpp>

#include <boost/program_options.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

#include <chrono>
#include <set>
#include <string>
#include <vector>

namespace
{
  std::vector<std::string>  test_nets
    { "net_with_single_module_implementation"
    , "net_with_multiple_module_implementations"
    };
}

BOOST_AUTO_TEST_CASE (steal_work)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::logging());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add_options()
    ( "test-net"
    , ::boost::program_options::value<std::string>()->required()
    , "name of the test pnet"
    );

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  auto const net (vm.at ("test-net").as<std::string>());

  gspc::util::temporary_path const shared_directory
    ( gspc::testing::shared_directory (vm)
    / ( "work_stealing"
      + net
      )
    );

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  gspc::util::temporary_path const tmp_dir
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  auto const installation_dir {std::filesystem::path {tmp_dir}};

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::testing::make_net_lib_install const make
    ( installation
    , net
    , gspc::testing::source_directory (vm)
    , installation_dir
    , gspc::testing::option::options()
    . add<gspc::testing::option::gen::include> (gspc::testing::source_directory (vm))
    . add<gspc::testing::option::gen::include>
        (gspc::testing::source_directory (vm).parent_path().parent_path().parent_path())
    );

  std::vector<std::string> const hosts
    (gspc::util::read_lines (nodefile_from_environment.path()));

  BOOST_REQUIRE_GT (hosts.size(), 0);

  gspc::scoped_rifds const rifds
    ( gspc::rifd::strategy {vm}
    , gspc::rifd::hostnames {vm}
    , gspc::rifd::port {vm}
    , installation
    );

  std::set<std::string> const capabilities {"A", "B", "C"};

  auto const num_workers_of_a_type_per_host
    (gspc::testing::random<std::size_t>{} (50, 10));

  std::ostringstream topology;
  gspc::util::first_then<std::string> sep ("", " ");

  for (auto const& capability : capabilities)
  {
    topology << sep << capability << ":" << num_workers_of_a_type_per_host;
  }

  gspc::scoped_runtime_system const drts
    (vm, installation, topology.str(), rifds.entry_points());

  long const num_tasks (6 * num_workers_of_a_type_per_host * hosts.size());
  auto worst_comp_time
    (gspc::testing::random<int>{} (10, 3));

  auto start = std::chrono::steady_clock::now();

  std::multimap<std::string, gspc::pnet::type::value::value_type> values_on_ports
    { {"num_tasks", num_tasks}
    , {"worst_comp_time", worst_comp_time}
    };

  if (net == test_nets.front())
  {
    values_on_ports.emplace ("slow_worker_host", hosts.front());
  }

  gspc::client (drts)
    .put_and_run (gspc::workflow (make.pnet()), values_on_ports);

  auto end = std::chrono::steady_clock::now();

  BOOST_REQUIRE_GE
    ( std::chrono::duration_cast<std::chrono::seconds> (end - start).count()
    , worst_comp_time
    );

  BOOST_REQUIRE_LE
    ( std::chrono::duration_cast<std::chrono::seconds> (end - start).count()
    , worst_comp_time + 1
    );
}
