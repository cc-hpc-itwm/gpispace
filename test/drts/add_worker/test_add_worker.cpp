// Copyright (C) 2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

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
#include <gspc/util/finally.hpp>
#include <gspc/util/read_lines.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/unique_path.hpp>

#include <gspc/testing/printer/we/type/value.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <future>
#include <iostream>
#include <list>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE (add_worker)
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
    / ( "add_worker"
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
    , "add_worker"
    , gspc::testing::source_directory (vm)
    , installation_dir
    );

  unsigned int n (3);

  std::list<gspc::scoped_rifds> rifds;

  std::vector<std::string> const hosts
    (gspc::util::read_lines (nodefile_from_environment.path()));

  BOOST_REQUIRE_GT (hosts.size(), 0);

  {
    auto host (hosts.begin());

    for (unsigned int i (0); i < n; ++i, ++host)
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
    (vm, installation, "worker:1", std::nullopt, parent.entry_point(), std::cerr, certificates);

  ::boost::asio::io_service io_service;
  ::boost::asio::io_service::work const work (io_service);

  ::boost::strict_scoped_thread<> const
    io_service_thread ([&io_service] { io_service.run(); });

  FHG_UTIL_FINALLY ([&] { io_service.stop(); });

  ::boost::asio::ip::tcp::acceptor acceptor (io_service, {});

  gspc::client client (drts, certificates);

  gspc::job_id_t const job_id
    ( client.submit
        ( gspc::workflow (make.pnet())
        , { {"address", gspc::util::connectable_to_address_string
                          (acceptor.local_endpoint().address())
            }
          , {"port", static_cast<unsigned int>
                       (acceptor.local_endpoint().port())
            }
          , {"wait", n}
          }
        )
    );

  {
    std::list<::boost::asio::ip::tcp::socket> connections;

    for (gspc::scoped_rifds const& rifd : rifds)
    {
      std::promise<void> connected;

      connections.emplace_back (io_service);
      acceptor.async_accept ( connections.back()
                            , [&connected] (::boost::system::error_code)
                              {
                                connected.set_value();
                              }
                            );

      drts.add_worker (rifd.entry_points(), certificates);

      client.put_token (job_id, "trigger", gspc::we::type::literal::control());

      connected.get_future().wait();
    }
  }

  auto const result (client.wait_and_extract (job_id));

  decltype (result) const expected {{"done", gspc::we::type::literal::control()}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}
