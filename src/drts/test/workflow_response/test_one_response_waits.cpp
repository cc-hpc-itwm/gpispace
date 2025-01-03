// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <test_start_callback.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_handler.hpp>
#include <util-rpc/service_tcp_provider.hpp>

#include <testing/certificates_data.hpp>
#include <testing/make.hpp>
#include <testing/parse_command_line.hpp>
#include <testing/scoped_nodefile_from_environment.hpp>
#include <testing/shared_directory.hpp>
#include <testing/source_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/starts_with.hpp>
#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>

#include <future>
#include <map>
#include <regex>

BOOST_AUTO_TEST_CASE (one_response_waits_while_others_are_made)
{
  ::boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add_options()
    ( "rpc-lib"
    , ::boost::program_options::value<::boost::filesystem::path>()->required()
    , "lib for workflow to link to"
    );
  options_description.add_options()
    ( "ssl-cert"
    , ::boost::program_options::value<std::string>()->required()
    , "enable or disable SSL certificate"
    );

  ::boost::program_options::variables_map vm
    ( test::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  std::string const ssl_cert (vm.at ("ssl-cert").as<std::string>());

  ::boost::filesystem::path test_directory
    ( test::shared_directory (vm)
    / ( "workflow_response_one_response_waits_while_others_are_made"
      + ssl_cert
      + "_cert"
      )
    );

  ::boost::filesystem::remove_all (test_directory);

  fhg::util::temporary_path const shared_directory (test_directory);

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
    , "workflow_response_one_response_waits_while_others_are_made"
    , test::source_directory (vm)
    , installation_dir
    , test::option::options()
    . add<test::option::gen::include> (test::source_directory (vm))
    . add<test::option::gen::include>
        (test::source_directory (vm).parent_path().parent_path().parent_path())
    . add<test::option::gen::link>
        (vm.at ("rpc-lib").as<::boost::filesystem::path>())
    . add<test::option::gen::ld_flag> ("-lboost_coroutine")
    . add<test::option::gen::ld_flag> ("-lboost_context")
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

  fhg::util::scoped_boost_asio_io_service_with_threads io_service (1);
  fhg::rpc::service_dispatcher service_dispatcher;
  fhg::rpc::service_handler<protocol::callback> register_service
    ( service_dispatcher
    , [&workflow_actually_running]
      {
        workflow_actually_running.set_value();
      }
    , fhg::rpc::not_yielding
    );
  fhg::rpc::service_tcp_provider const registry (io_service, service_dispatcher);

  gspc::job_id_t const job_id
    ( client.submit
        ( workflow
        , { {"state", initial_state}
          , {"register_host", fhg::util::connectable_to_address_string
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
                         (job_id, "done_done", we::type::literal::control());
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

    fhg::util::wait_and_collect_exceptions (results);
  }

  done_check.get();

  std::multimap<std::string, pnet::type::value::value_type> const result
    (client.extract_result_and_forget_job (job_id));

  std::string const port_done ("done");
  std::string const port_state ("state");

  BOOST_REQUIRE_EQUAL (result.count (port_done), 1);
  BOOST_REQUIRE_EQUAL
    ( result.find (port_done)->second
    , pnet::type::value::value_type (we::type::literal::control())
    );
  BOOST_REQUIRE_EQUAL (result.count (port_state), 1);
  BOOST_REQUIRE_EQUAL ( result.find (port_state)->second
                      , pnet::type::value::value_type
                          (initial_state + status_updates)
                      );
}
