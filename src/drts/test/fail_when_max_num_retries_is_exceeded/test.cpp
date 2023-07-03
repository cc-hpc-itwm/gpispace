// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <drts/test/fail_when_max_num_retries_is_exceeded/test_callback.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <testing/certificates_data.hpp>
#include <testing/make.hpp>
#include <testing/parse_command_line.hpp>
#include <testing/scoped_nodefile_from_environment.hpp>
#include <testing/shared_directory.hpp>
#include <testing/source_directory.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>

#include <util-generic/threadsafe_queue.hpp>
#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_handler.hpp>
#include <util-rpc/service_tcp_provider.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <stdexcept>
#include <string>

BOOST_AUTO_TEST_CASE (fail_when_max_num_retries_is_exceeded)
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
    , "rpc library to link against"
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

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / ( "fail_when_max_num_retries_is_exceeded"
      + ssl_cert
      + "_cert"
      )
    );

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
    , "fail_when_max_num_retries_is_exceeded"
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

  gspc::scoped_runtime_system drts
    (vm, installation, "worker:1", rifds.entry_points(), std::cerr, certificates);

  gspc::client client (drts, certificates);

  struct Empty {};

  fhg::util::threadsafe_queue<Empty> job_submitted;
  fhg::util::scoped_boost_asio_io_service_with_threads io_service (1);
  fhg::rpc::service_dispatcher service_dispatcher;
  fhg::rpc::service_handler<protocol::callback> register_service
    ( service_dispatcher
    , [&] ()
      {
        job_submitted.put (Empty{});
      }
    , fhg::rpc::not_yielding
    );

  fhg::rpc::service_tcp_provider const registry (io_service, service_dispatcher);

  gspc::job_id_t const job_id
    ( client.submit
        ( gspc::workflow (make.pnet())
        , { {"host", fhg::util::connectable_to_address_string
                       (registry.local_endpoint().address())}
          , {"port", static_cast<unsigned int>
                       (registry.local_endpoint().port())}
          , {"start", we::type::literal::control()}
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
