// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <network_description.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <testing/make.hpp>
#include <testing/parse_command_line.hpp>
#include <testing/scoped_nodefile_from_environment.hpp>
#include <testing/shared_directory.hpp>
#include <testing/source_directory.hpp>
#include <testing/virtual_memory_socket_name_for_localhost.hpp>

#include <util-generic/boost/program_options/validators/positive_integral.hpp>

#include <util-generic/read_lines.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>
#include <util-generic/testing/require_maximum_running_time.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <chrono>
#include <string>
#include <vector>

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
  namespace validators = fhg::util::boost::program_options;

  ::boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
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
    ( test::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  long const num_tasks
    (vm.at ("num-tasks").as<validators::positive_integral<long>>());
  long const num_workers
    (vm.at ("num-workers").as<validators::positive_integral<long>>());

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / "scheduler_performance"
    );

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / ::boost::filesystem::unique_path());
  ::boost::filesystem::path const installation_dir (_installation_dir);

  fhg::util::temporary_path const _workflow_dir
    (shared_directory / ::boost::filesystem::unique_path());
  ::boost::filesystem::path const workflow_dir (_workflow_dir);

  ::boost::filesystem::ofstream (workflow_dir / "net.xpnet")
    << drts::test::create_network_description ("net", boost::none, boost::none);

  boost::optional<std::string> with_num_worker_prop
    { R"EOS(
        <properties name="fhg">
          <properties name="drts">
            <properties name="schedule">
              <property key="num_worker">"1UL"</property>
            </properties>
          </properties>
        </properties>)EOS"
    };

  ::boost::filesystem::ofstream (workflow_dir / "net_with_num_workers_prop.xpnet")
    << drts::test::create_network_description
         ("net_with_num_workers_prop", with_num_worker_prop, boost::none);

  boost::optional<std::string> with_memory_buffers
    { R"EOS(
        <memory-buffer name="local">
          <size>
            0UL
          </size>
        </memory-buffer>)EOS"
    };

  ::boost::filesystem::ofstream (workflow_dir / "net_with_memory_buffers.xpnet")
    << drts::test::create_network_description
         ("net_with_memory_buffers", boost::none, with_memory_buffers);

  gspc::set_application_search_path (vm, installation_dir);
  test::set_virtual_memory_socket_name_for_localhost (vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net_lib_install const make_net
    ( installation
    , "net"
    , workflow_dir
    , installation_dir
    , test::option::options()
    . add<test::option::gen::include> (test::source_directory (vm))
    . add<test::option::gen::include>
        (test::source_directory (vm).parent_path().parent_path().parent_path())
    );

  test::make_net_lib_install const make_net_with_num_workers_prop
    ( installation
    , "net_with_num_workers_prop"
    , workflow_dir
    , installation_dir
    , test::option::options()
    . add<test::option::gen::include> (test::source_directory (vm))
    . add<test::option::gen::include>
        (test::source_directory (vm).parent_path().parent_path().parent_path())
    );

  test::make_net_lib_install const make_net_with_memory_buffers
    ( installation
    , "net_with_memory_buffers"
    , workflow_dir
    , installation_dir
    , test::option::options()
    . add<test::option::gen::include> (test::source_directory (vm))
    . add<test::option::gen::include>
        (test::source_directory (vm).parent_path().parent_path().parent_path())
    );

  std::vector<std::string> const hosts
    (fhg::util::read_lines (nodefile_from_environment.path()));

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

            decltype (result) const expected {{"done", we::type::literal::control()}};
            FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
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

            decltype (result) const expected {{"done", we::type::literal::control()}};
            FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
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

            decltype (result) const expected {{"done", we::type::literal::control()}};
            FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
          }
       ).count()
    );

  BOOST_REQUIRE_LE (elapsed_single_allocation, elapsed_coallocation);
  BOOST_REQUIRE_LE (elapsed_greedy, elapsed_single_allocation);
}
