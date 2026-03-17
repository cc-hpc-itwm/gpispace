// Copyright (C) 2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <test/drts/fail_when_max_num_retries_is_exceeded/test_callback.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <gspc/testing/certificates_data.hpp>
#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/source_directory.hpp>

#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>
#include <gspc/util/temporary_file.hpp>
#include <gspc/util/temporary_path.hpp>

#include <gspc/util/threadsafe_queue.hpp>
#include <gspc/testing/unique_path.hpp>
#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_tcp_provider.hpp>

#include <gspc/testing/printer/we/type/value.hpp>

#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <stdexcept>
#include <string>

BOOST_AUTO_TEST_CASE (fail_when_max_num_retries_is_exceeded)
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
    / ( "fail_when_max_num_retries_is_exceeded"
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
    , "fail_when_max_num_retries_is_exceeded"
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

  gspc::scoped_runtime_system drts
    (vm, installation, "worker:1", rifds.entry_points(), std::cerr, certificates);

  gspc::client client (drts, certificates);

  struct Empty {};

  gspc::util::threadsafe_queue<Empty> job_submitted;
  gspc::util::scoped_boost_asio_io_service_with_threads io_service (1);
  gspc::rpc::service_dispatcher service_dispatcher;
  gspc::rpc::service_handler<protocol::callback> register_service
    ( service_dispatcher
    , [&] ()
      {
        job_submitted.put (Empty{});
      }
    , gspc::rpc::not_yielding
    );

  gspc::rpc::service_tcp_provider const registry (io_service, service_dispatcher);

  gspc::job_id_t const job_id
    ( client.submit
        ( gspc::workflow (make.pnet())
        , { {"host", gspc::util::connectable_to_address_string
                       (registry.local_endpoint().address())}
          , {"port", static_cast<unsigned int>
                       (registry.local_endpoint().port())}
          , {"start", gspc::we::type::literal::control()}
          }
        )
    );

  job_submitted.get();
  drts.remove_worker (rifds.entry_points());

  drts.add_worker (rifds.entry_points(), certificates);

  job_submitted.get();
  drts.remove_worker (rifds.entry_points());

  BOOST_REQUIRE_EXCEPTION
    ( client.wait (job_id)
    , std::runtime_error
    , [&] (std::runtime_error const& exc)
      {
        std::string error (exc.what());
        std::string expected ("Number of retries exceeded!");
        return error.find (expected) != std::string::npos;
      }
    );
}
