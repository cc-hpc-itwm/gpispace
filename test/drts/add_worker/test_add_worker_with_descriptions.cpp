// Copyright (C) 2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <test/drts/add_worker/test_callback.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_tcp_provider.hpp>

#include <gspc/testing/certificates_data.hpp>
#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/source_directory.hpp>

#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/read_lines.hpp>
#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/unique_path.hpp>

#include <gspc/testing/printer/we/type/value.hpp>

#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>

#include <fmt/core.h>
#include <iostream>
#include <list>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
  void store_expected_worker
    ( std::filesystem::path const& test_dir
    , gspc::scoped_rifds const& rifd
    , std::string const& capability
    , std::set<std::string>& expected_workers
    )
  {
    std::filesystem::path const ep_file (test_dir / "tmp_entry_point.txt");
    rifd.entry_points().write_to_file (ep_file);

    std::vector<std::string> const entrypoints (gspc::util::read_lines (ep_file));
    BOOST_REQUIRE_EQUAL (entrypoints.size(), 1);

    expected_workers.emplace (capability + "-" + entrypoints.at (0) + "-1");
  }
}

// Note: this test checks the exact identity of the worker executing
// a task that requires a specific capability. However, for the purpose of
// this test it is sufficient to test only if a worker with a given capability
// executed a task requiring that capability. This would make the test
// invariant to subsequent changes in the worker naming convention.
BOOST_AUTO_TEST_CASE (add_workers_with_different_descriptions)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::logging());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add_options()
    ( "rpc-lib"
    , ::boost::program_options::value<std::filesystem::path>()->required()
    , "rpc library to link against"
    );
  options_description.add_options()
    ( "ssl-cert"
    , ::boost::program_options::value<std::string>()->required()
    , "enable or disable SSL certificate"
    );

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  std::string const ssl_cert (vm.at ("ssl-cert").as<std::string>());

  gspc::util::temporary_path const shared_directory
    ( gspc::testing::shared_directory (vm)
    / ( "add_worker_with_different_descriptions"
      + ssl_cert
      + "_cert"
      )
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
    , "add_workers_with_different_descriptions"
    , gspc::testing::source_directory (vm)
    , installation_dir
    , gspc::testing::option::options()
    . add<gspc::testing::option::gen::include> (gspc::testing::source_directory (vm))
    . add<gspc::testing::option::gen::include>
     (gspc::testing::source_directory (vm).parent_path().parent_path().parent_path())
    . add<gspc::testing::option::gen::link>
     (vm.at ("rpc-lib").as<std::filesystem::path>())
    . add<gspc::testing::option::gen::ld_flag> ("-lboost_coroutine")
    . add<gspc::testing::option::gen::ld_flag> ("-lboost_context")
    );

  std::vector<std::string> const capabilities {"A", "B", "C"};

  std::list<gspc::scoped_rifds> rifds;

  std::vector<std::string> const hosts
    (gspc::util::read_lines (nodefile_from_environment.path()));

  BOOST_REQUIRE_GT (hosts.size(), 0);

  {
    auto host (hosts.begin());

    for (unsigned int i (0); i < capabilities.size(); ++i, ++host)
    {
      if (host == hosts.end())
      {
        host = hosts.begin();
      }

      rifds.emplace_back ( gspc::rifd::strategy (vm)
                         , gspc::rifd::hostnames ({*host})
                         , gspc::rifd::port (vm)
                         , installation
                         );
    }
  }

  gspc::scoped_rifd const parent
    ( gspc::rifd::strategy {vm}
    , gspc::rifd::hostname {hosts.front()}
    , gspc::rifd::port {vm}
    , installation
    );

  auto const certificates ( ssl_cert  == "yes" ? gspc::testing::yes_certs()
                                               : gspc::testing::no_certs()
                          );

  gspc::scoped_runtime_system drts
    (vm, installation, "", std::nullopt, parent.entry_point(), std::cerr, certificates);

  std::set<std::string> expected_workers;

  unsigned int k {0};
  for (gspc::scoped_rifds const& rifd : rifds)
  {
    store_expected_worker
      (shared_directory, rifd, capabilities[k], expected_workers);

    std::vector<gspc::worker_description> descriptions;
    descriptions.emplace_back
      (std::vector<std::string> {capabilities[k++]}, 1, 0, 0, std::nullopt, std::nullopt);
    drts.add_worker (descriptions, rifd.entry_points(), certificates);
  }

  gspc::client client (drts, certificates);

  std::set<std::string> announced_workers;
  gspc::util::scoped_boost_asio_io_service_with_threads io_service (1);
  gspc::rpc::service_dispatcher service_dispatcher;
  gspc::rpc::service_handler<protocol::callback> register_service
    ( service_dispatcher
    , [&announced_workers] (std::string w)
      {
        if (!announced_workers.emplace (w).second)
        {
          throw std::runtime_error {fmt::format ("Duplicate worker '{}'", w)};
        }
      }
    , gspc::rpc::not_yielding
    );

  gspc::rpc::service_tcp_provider const registry (io_service, service_dispatcher);

  std::multimap<std::string, gspc::pnet::type::value::value_type> const result
    ( client.put_and_run
        ( gspc::workflow (make.pnet())
        , { {"host", gspc::util::connectable_to_address_string
                       (registry.local_endpoint().address())}
          , {"port", static_cast<unsigned int>
                        (registry.local_endpoint().port())}
          , {"start", true}
          }
        )
    );


  BOOST_REQUIRE_EQUAL (announced_workers.size(), capabilities.size());

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    ( announced_workers.begin()
    , announced_workers.end()
    , expected_workers.begin()
    , expected_workers.end()
    );

  decltype (result) const expected {{"all_done", gspc::we::type::literal::control()}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}
