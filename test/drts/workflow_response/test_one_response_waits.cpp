// Copyright (C) 2020-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <test_start_callback.hpp>

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

#include <gspc/we/type/value.hpp>
#include <gspc/testing/printer/we/type/value.hpp>

#include <gspc/util/starts_with.hpp>
#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/optional.hpp>
#include <gspc/testing/require_exception.hpp>
#include <gspc/testing/unique_path.hpp>
#include <gspc/util/wait_and_collect_exceptions.hpp>

#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>

#include <future>
#include <map>
#include <regex>

BOOST_AUTO_TEST_CASE (one_response_waits_while_others_are_made)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add_options()
    ( "rpc-lib"
    , ::boost::program_options::value<std::filesystem::path>()->required()
    , "lib for workflow to link to"
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

  std::filesystem::path test_directory
    ( gspc::testing::shared_directory (vm)
    / ( "workflow_response_one_response_waits_while_others_are_made"
      + ssl_cert
      + "_cert"
      )
    );

  std::filesystem::remove_all (test_directory);

  gspc::util::temporary_path const shared_directory (test_directory);

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
    , "workflow_response_one_response_waits_while_others_are_made"
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

  gspc::scoped_rifds const rifds { gspc::rifd::strategy (vm)
                                 , gspc::rifd::hostnames (vm)
                                 , gspc::rifd::port (vm)
                                 , installation
                                 };

  auto const certificates ( ssl_cert  == "yes" ? gspc::testing::yes_certs()
                                               : gspc::testing::no_certs()
                          );

  gspc::scoped_runtime_system const drts
    (vm, installation, "work:2 management:1", rifds.entry_points(), std::cerr, certificates);

  gspc::client client (drts, certificates);

  gspc::workflow workflow (make.pnet());

  workflow.set_wait_for_output();

  unsigned long const initial_state (0);
  std::atomic<unsigned long> status_updates (0);
  unsigned long const threads (2);

  std::promise<void> workflow_actually_running;

  gspc::util::scoped_boost_asio_io_service_with_threads io_service (1);
  gspc::rpc::service_dispatcher service_dispatcher;
  gspc::rpc::service_handler<protocol::callback> register_service
    ( service_dispatcher
    , [&workflow_actually_running]
      {
        workflow_actually_running.set_value();
      }
    , gspc::rpc::not_yielding
    );
  gspc::rpc::service_tcp_provider const registry (io_service, service_dispatcher);

  gspc::job_id_t const job_id
    ( client.submit
        ( workflow
        , { {"state", initial_state}
          , {"register_host", gspc::util::connectable_to_address_string
                                (registry.local_endpoint().address())}
          , {"register_port", static_cast<unsigned int>
                                (registry.local_endpoint().port())}
          }
        )
    );

  workflow_actually_running.get_future().wait();

  std::mutex no_longer_do_status_update_guard;
  bool no_longer_do_status_update (false);

  std::future<void> done_check
    ( std::async ( std::launch::async
                 , [ &client, &job_id, &no_longer_do_status_update
                   , &no_longer_do_status_update_guard
                   ]
                   {
                     client.synchronous_workflow_response
                       (job_id, "check_done_trigger", 0UL);

                     no_longer_do_status_update = true;
                     {
                       std::lock_guard<std::mutex> const _
                         (no_longer_do_status_update_guard);

                       client.put_token
                         (job_id, "done_done", gspc::we::type::literal::control());
                     }

                     client.wait (job_id);
                   }
                 )
    );

  auto&& thread_function
    ( [ &status_updates, &job_id, &drts, &no_longer_do_status_update
      , &no_longer_do_status_update_guard, &certificates
      ]
      {
        unsigned long updates (0);
        while (true)
        {
          std::lock_guard<std::mutex> const _ (no_longer_do_status_update_guard);
          if (no_longer_do_status_update)
          {
            break;
          }

          gspc::client (drts, certificates).synchronous_workflow_response
            (job_id, "get_and_update_state_trigger", 1UL);
          ++updates;
        }
        status_updates += updates;
      }
    );

  {
    std::vector<std::future<void>> results;
    for (unsigned long i (0); i < threads; ++i)
    {
      results.emplace_back (std::async (std::launch::async, thread_function));
    }

    gspc::util::wait_and_collect_exceptions (results);
  }

  done_check.get();

  std::multimap<std::string, gspc::pnet::type::value::value_type> const result
    (client.extract_result_and_forget_job (job_id));

  std::string const port_done ("done");
  std::string const port_state ("state");

  BOOST_REQUIRE_EQUAL (result.count (port_done), 1);
  BOOST_REQUIRE_EQUAL
    ( result.find (port_done)->second
    , gspc::pnet::type::value::value_type (gspc::we::type::literal::control())
    );
  BOOST_REQUIRE_EQUAL (result.count (port_state), 1);
  BOOST_REQUIRE_EQUAL ( result.find (port_state)->second
                      , gspc::pnet::type::value::value_type
                          (initial_state + status_updates)
                      );
}
