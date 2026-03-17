// Copyright (C) 2021-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <test/parallel_reduce/module_call/JobServer.hpp>

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
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>

#include <fmt/core.h>
#include <algorithm>
#include <utility>
#include <vector>

namespace
{
  std::vector<unsigned long> elements()
  {
    return {1,10,100};
  }
  std::vector<unsigned long> workers()
  {
    return {1,5};
  }
}

BOOST_DATA_TEST_CASE
  ( parallel_reduce_manual_module_call
  , elements() * workers()
  , n
  , number_of_workers_per_host
  )
{
  boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::logging());
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
    (gspc::testing::shared_directory (vm) / "parallel_reduce_module_call");

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
    , "manual"
    , gspc::testing::source_directory (vm)
    , installation_dir
    , gspc::testing::option::options()
    . add<gspc::testing::option::gen::include>
      ( gspc::testing::source_directory (vm)
      . parent_path() // module_call
      . parent_path() // parallel_reduce
      . parent_path() // test
      )
    . add<gspc::testing::option::gen::include>
      ( gspc::testing::source_directory (vm)
      . parent_path()
      . parent_path()
      . parent_path()
      / "src"
      )
    . add<gspc::testing::option::gen::link> (std::filesystem::path {RPC_LIB})
    . add<gspc::testing::option::gen::link> (std::filesystem::path {JOB_EXECUTOR_LIB})
    . add<gspc::testing::option::gen::link> (std::filesystem::path {TASK_LIB})
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  gspc::scoped_runtime_system const drts
    ( vm
    , installation
    , fmt::format ("work:{}", number_of_workers_per_host)
    , rifds.entry_points()
    );

  unsigned long const number_of_workers
    {number_of_workers_per_host * rifds.hosts().size()};

  ::gspc::test::parallel_reduce::module_call::JobServer job_server
    {n, number_of_workers};
  auto const address (job_server.address());

  gspc::client client (drts);

  auto const job_id
    ( client.submit
        ( gspc::workflow (make.pnet())
        , { {"n", n}
          , {"host", address.first}
          , {"port", static_cast<unsigned int> (address.second)}
          }
        )
    );

  auto const max_parallel_running_tasks
    (std::move (job_server).wait_and_return_max_parallel_running_tasks());

  auto const result (client.wait_and_extract (job_id));

  gspc::pnet::type::value::value_type P;
  gspc::pnet::type::value::poke ("value", P, (n + 1ul) * n / 2ul);
  decltype (result) const expected {{"P", P}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);

   BOOST_REQUIRE
     (max_parallel_running_tasks == std::min (n / 2, number_of_workers));
}
