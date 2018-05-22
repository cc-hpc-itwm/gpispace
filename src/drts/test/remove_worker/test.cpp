#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/finally.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/thread/event.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/scoped_thread.hpp>

namespace
{
  gspc::certificates_t const test_certificates (GSPC_SSL_CERTIFICATES_FOR_TESTS);
}

void test_remove_worker (gspc::certificates_t const& certificates)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::logging());
  options_description.add (gspc::options::scoped_rifd());

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "remove_worker");

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
    , "remove_worker"
    , test::source_directory (vm)
    , installation_dir
    );

  gspc::scoped_rifds const rifds { gspc::rifd::strategy (vm)
                                 , gspc::rifd::hostnames (vm)
                                 , gspc::rifd::port (vm)
                                 , installation
                                 };

  gspc::scoped_runtime_system drts
    (vm, installation, "worker:1", rifds.entry_points(), certificates);

  boost::asio::io_service io_service;
  boost::asio::io_service::work const work (io_service);

  boost::strict_scoped_thread<> const
    io_service_thread ([&io_service] { io_service.run(); });

  FHG_UTIL_FINALLY ([&] { io_service.stop(); });

  boost::asio::ip::tcp::acceptor acceptor (io_service, {});
  boost::asio::ip::tcp::socket connection (io_service);
  fhg::util::thread::event<> connected;

  acceptor.async_accept ( connection
                        , [&connected] (boost::system::error_code)
                          {
                            connected.notify();
                          }
                        );

  gspc::job_id_t const job_id
    ( gspc::client (drts, certificates).submit
        ( gspc::workflow (make.pnet())
        , { {"address", fhg::util::connectable_to_address_string
                          (acceptor.local_endpoint().address())
            }
          , {"port", static_cast<unsigned int>
                       (acceptor.local_endpoint().port())
            }
          }
        )
    );

  connected.wait();

  drts.remove_worker (rifds.entry_points());

  boost::system::error_code errc;
  char buffer;
  BOOST_REQUIRE_EQUAL
    (0, boost::asio::read (connection, boost::asio::buffer (&buffer, 1), errc));

  BOOST_REQUIRE ( errc == boost::asio::error::eof
                || errc == boost::asio::error::connection_reset
                );
}

BOOST_AUTO_TEST_CASE (remove_worker)
{
  test_remove_worker (boost::none);
}

BOOST_AUTO_TEST_CASE (remove_secure_worker)
{
  test_remove_worker (test_certificates);
}
