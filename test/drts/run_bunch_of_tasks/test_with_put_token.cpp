// Copyright (C) 2020-2024,2026 Fraunhofer ITWM
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

#include <gspc/util/boost/program_options/validators/positive_integral.hpp>

#include <gspc/util/read_lines.hpp>
#include <gspc/util/temporary_file.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/require_maximum_running_time.hpp>
#include <gspc/testing/unique_path.hpp>

#include <gspc/testing/printer/we/type/value.hpp>

#include <boost/program_options.hpp>

#include <chrono>
#include <iterator>
#include <set>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE (run_bunch_of_tasks_with_put_token)
{
  namespace validators = gspc::util::boost::program_options;

  ::boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
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
    / "run_bunch_of_tasks_with_put_token"
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
    , "run_bunch_of_tasks_with_put_token"
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

  gspc::scoped_runtime_system const drts
    ( vm
    , installation
    , "A:" + std::to_string (num_workers)
    , rifds.entry_points()
    , std::cerr
    , {}
    );

  gspc::client client (drts);
  gspc::we::type::literal::control start;

  GSPC_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME (std::chrono::seconds (35))
  {
    auto const job_id
      (client.submit
         ( gspc::workflow (make.pnet())
         , {{"start", start}, {"num_tasks", num_tasks}}
         )
      );

    for (unsigned int i {0}; i < num_tasks - 1; i++)
    {
      client.put_token (job_id, "start", start);
    }

    auto const result (client.wait_and_extract (job_id));
    decltype (result) const expected {{"done", gspc::we::type::literal::control()}};
    GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
  };
}
