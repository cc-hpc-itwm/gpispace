// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <test/parallel_reduce/module_call/JobServer.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>
#include <we/type/value/poke.hpp>

#include <testing/make.hpp>
#include <testing/parse_command_line.hpp>
#include <testing/scoped_nodefile_from_environment.hpp>
#include <testing/shared_directory.hpp>
#include <testing/source_directory.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

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

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::logging());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "parallel_reduce_module_call");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net_lib_install const make
    ( installation
    , "manual"
    , test::source_directory (vm)
    , installation_dir
    , test::option::options()
    . add<test::option::gen::include>
      ( test::source_directory (vm)
      . parent_path() // module_call
      . parent_path() // parallel_reduce
      . parent_path() // test
      )
    . add<test::option::gen::include>
      ( test::source_directory (vm)
      . parent_path()
      . parent_path()
      . parent_path()
      / "src"
      )
    . add<test::option::gen::link> (::boost::filesystem::path {RPC_LIB})
    . add<test::option::gen::link> (::boost::filesystem::path {JOB_EXECUTOR_LIB})
    . add<test::option::gen::link> (::boost::filesystem::path {TASK_LIB})
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

  ::pnet::type::value::value_type P;
  ::pnet::type::value::poke ("value", P, (n + 1ul) * n / 2ul);
  decltype (result) const expected {{"P", P}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);

   BOOST_REQUIRE
     (max_parallel_running_tasks == std::min (n / 2, number_of_workers));
}
