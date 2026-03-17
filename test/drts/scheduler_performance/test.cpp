// Copyright (C) 2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <network_description.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/source_directory.hpp>
#include <gspc/testing/virtual_memory_socket_name_for_localhost.hpp>

#include <gspc/util/boost/program_options/validators/positive_integral.hpp>

#include <gspc/util/read_lines.hpp>
#include <gspc/util/temporary_file.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/require_maximum_running_time.hpp>
#include <gspc/testing/unique_path.hpp>

#include <gspc/testing/printer/we/type/value.hpp>

#include <optional>
#include <boost/program_options.hpp>

#include <chrono>
#include <string>
#include <vector>
#include <fstream>

namespace
{
  template< typename F
          , typename Clock = std::chrono::steady_clock
          , typename Duration = std::chrono::milliseconds
          >
    Duration duration (F&& f)
  {
    auto const start {Clock::now()};

    f();

    return std::chrono::duration_cast<Duration> (Clock::now() - start);
  }
}

BOOST_AUTO_TEST_CASE (scheduler_performance)
{
  namespace validators = gspc::util::boost::program_options;

  ::boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (gspc::options::virtual_memory());
  options_description.add_options()
    ( "num-workers"
    , ::boost::program_options::value<validators::positive_integral<long>>()->required()
    , "number of workers"
    );
  options_description.add_options()
    ( "num-tasks"
    , ::boost::program_options::value<validators::positive_integral<long>>()->required()
    , "number of tasks"
    );

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  long const num_tasks
    (vm.at ("num-tasks").as<validators::positive_integral<long>>());
  long const num_workers
    (vm.at ("num-workers").as<validators::positive_integral<long>>());

  gspc::util::temporary_path const shared_directory
    ( gspc::testing::shared_directory (vm)
    / "scheduler_performance"
    );

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  gspc::util::temporary_path const _installation_dir
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  auto const installation_dir {std::filesystem::path {_installation_dir}};

  gspc::util::temporary_path const _workflow_dir
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  auto const workflow_dir {std::filesystem::path {_workflow_dir}};

  std::ofstream (workflow_dir / "net.xpnet")
    << drts::test::create_network_description ("net", std::nullopt, std::nullopt);

  std::optional<std::string> with_num_worker_prop
    { R"EOS(
        <properties name="fhg">
          <properties name="drts">
            <properties name="schedule">
              <property key="num_worker">"1UL"</property>
            </properties>
          </properties>
        </properties>)EOS"
    };

  std::ofstream (workflow_dir / "net_with_num_workers_prop.xpnet")
    << drts::test::create_network_description
         ("net_with_num_workers_prop", with_num_worker_prop, std::nullopt);

  std::optional<std::string> with_memory_buffers
    { R"EOS(
        <memory-buffer name="local">
          <size>
            0UL
          </size>
        </memory-buffer>)EOS"
    };

  std::ofstream (workflow_dir / "net_with_memory_buffers.xpnet")
    << drts::test::create_network_description
         ("net_with_memory_buffers", std::nullopt, with_memory_buffers);

  gspc::set_application_search_path (vm, installation_dir);
  gspc::testing::set_virtual_memory_socket_name_for_localhost (vm);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::testing::make_net_lib_install const make_net
    ( installation
    , "net"
    , workflow_dir
    , installation_dir
    , gspc::testing::option::options()
    . add<gspc::testing::option::gen::include> (gspc::testing::source_directory (vm))
    . add<gspc::testing::option::gen::include>
        (gspc::testing::source_directory (vm).parent_path().parent_path().parent_path())
    );

  gspc::testing::make_net_lib_install const make_net_with_num_workers_prop
    ( installation
    , "net_with_num_workers_prop"
    , workflow_dir
    , installation_dir
    , gspc::testing::option::options()
    . add<gspc::testing::option::gen::include> (gspc::testing::source_directory (vm))
    . add<gspc::testing::option::gen::include>
        (gspc::testing::source_directory (vm).parent_path().parent_path().parent_path())
    );

  gspc::testing::make_net_lib_install const make_net_with_memory_buffers
    ( installation
    , "net_with_memory_buffers"
    , workflow_dir
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

  gspc::scoped_runtime_system const drts
    ( vm
    , installation
    , "A:" + std::to_string (num_workers) + ",100 " + "B:" + std::to_string (num_workers) + ",100 "
    , rifds.entry_points()
    );

  gspc::client client (drts);

  auto const elapsed_coallocation
    ( duration
        ( [&]
          {
            auto const result
              ( client.put_and_run
                  ( gspc::workflow (make_net_with_num_workers_prop.pnet())
                  , {{"num_tasks", num_tasks}}
                  )
              );

            decltype (result) const expected {{"done", gspc::we::type::literal::control()}};
            GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
          }
       ).count()
    );

  auto const elapsed_single_allocation
    ( duration
        ( [&]
          {
            auto const result
              ( client.put_and_run
                  ( gspc::workflow (make_net_with_memory_buffers.pnet())
                  , {{"num_tasks", num_tasks}}
                  )
              );

            decltype (result) const expected {{"done", gspc::we::type::literal::control()}};
            GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
          }
       ).count()
    );

  auto const elapsed_greedy
    ( duration
        ( [&]
          {
            auto const result
              ( client.put_and_run
                  ( gspc::workflow (make_net.pnet())
                  , {{"num_tasks", num_tasks}}
                  )
              );

            decltype (result) const expected {{"done", gspc::we::type::literal::control()}};
            GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
          }
       ).count()
    );

  BOOST_REQUIRE_LE (elapsed_single_allocation, elapsed_coallocation);
  BOOST_REQUIRE_LE (elapsed_greedy, elapsed_single_allocation);
}
