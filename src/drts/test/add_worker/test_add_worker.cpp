#include <boost/test/unit_test.hpp>

#include <test_callback.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>
#include <rpc/service_tcp_provider.hpp>

#include <test/certificates_data.hpp>
#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <fhg/util/thread/event.hpp>
#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/finally.hpp>
#include <util-generic/read_lines.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <list>
#include <vector>

BOOST_AUTO_TEST_CASE (add_worker)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::logging());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add_options()
    ( "rpc-lib"
    , boost::program_options::value<boost::filesystem::path>()->required()
    , "rpc library to link against"
    );
  options_description.add_options()
    ( "ssl-cert"
    , boost::program_options::value<std::string>()->required()
    , "enable or disable SSL certificate"
    );

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "add_worker");

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
    , "add_worker"
    , test::source_directory (vm)
    , installation_dir
    );

  unsigned int n (3);

  std::list<gspc::scoped_rifds> rifds;

  std::vector<std::string> const hosts
    (fhg::util::read_lines (nodefile_from_environment.path()));

  BOOST_REQUIRE_GT (hosts.size(), 0);

  {
    std::vector<std::string>::const_iterator host (hosts.begin());

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

  gspc::scoped_rifd const master
    ( gspc::rifd::strategy {vm}
    , gspc::rifd::hostname {hosts.front()}
    , gspc::rifd::port {vm}
    , installation
    );

  std::string const ssl_cert (vm.at ("ssl-cert").as<std::string>());
  auto const certificates ( ssl_cert  == "yes" ? gspc::testing::yes_certs()
                                               : gspc::testing::no_certs()
                          );

  gspc::scoped_runtime_system drts
    (vm, installation, "worker:1", boost::none, master.entry_point(), std::cerr, certificates);

  boost::asio::io_service io_service;
  boost::asio::io_service::work const work (io_service);

  boost::strict_scoped_thread<> const
    io_service_thread ([&io_service] { io_service.run(); });

  FHG_UTIL_FINALLY ([&] { io_service.stop(); });

  boost::asio::ip::tcp::acceptor acceptor (io_service, {});

  gspc::client client (drts, certificates);

  gspc::job_id_t const job_id
    ( client.submit
        ( gspc::workflow (make.pnet())
        , { {"address", fhg::util::connectable_to_address_string
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
    std::list<boost::asio::ip::tcp::socket> connections;
    fhg::util::thread::event<> connected;

    for (gspc::scoped_rifds const& rifd : rifds)
    {
      connections.emplace_back (io_service);
      acceptor.async_accept ( connections.back()
                            , [&connected] (boost::system::error_code)
                              {
                                connected.notify();
                              }
                            );

      drts.add_worker (rifd.entry_points(), certificates);

      client.put_token (job_id, "trigger", we::type::literal::control());

      connected.wait();
    }
  }

  std::multimap<std::string, pnet::type::value::value_type> const result
    (client.wait_and_extract (job_id));

  decltype (result) const expected {{"done", we::type::literal::control()}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}
