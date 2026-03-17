// Copyright (C) 2015-2016,2018-2023,2025-2026 Fraunhofer ITWM
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
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/optional.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <future>
#include <utility>

BOOST_AUTO_TEST_CASE (remove_worker)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::logging());
  options_description.add (gspc::options::scoped_rifd());
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
    / ( "remove_worker"
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
    , "remove_worker"
    , gspc::testing::source_directory (vm)
    , installation_dir
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

  ::boost::asio::io_service io_service;
  ::boost::asio::io_service::work const work (io_service);

  ::boost::strict_scoped_thread<> const
    io_service_thread ([&io_service] { io_service.run(); });

  FHG_UTIL_FINALLY ([&] { io_service.stop(); });

  ::boost::asio::ip::tcp::acceptor acceptor (io_service, {});
  ::boost::asio::ip::tcp::socket connection (io_service);
  std::promise<void> connected;

  acceptor.async_accept ( connection
                        , [&connected] (::boost::system::error_code)
                          {
                            connected.set_value();
                          }
                        );

  std::ignore =
      gspc::client (drts, certificates).submit
        ( gspc::workflow (make.pnet())
        , { {"address", gspc::util::connectable_to_address_string
                          (acceptor.local_endpoint().address())
            }
          , {"port", static_cast<unsigned int>
                       (acceptor.local_endpoint().port())
            }
          }
        );

  connected.get_future().wait();

  drts.remove_worker (rifds.entry_points());

  ::boost::system::error_code errc;
  char buffer;
  BOOST_REQUIRE_EQUAL
    (0, ::boost::asio::read (connection, ::boost::asio::buffer (&buffer, 1), errc));

  BOOST_REQUIRE ( errc == ::boost::asio::error::eof
                || errc == ::boost::asio::error::connection_reset
                );
}
