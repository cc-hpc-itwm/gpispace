#define BOOST_TEST_MODULE drts_remove_worker
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <network/server.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <network/connectable_to_address_string.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/boost/test/require_exception.hpp>
#include <fhg/util/temporary_path.hpp>
#include <fhg/util/thread/event.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/scoped_thread.hpp>

BOOST_AUTO_TEST_CASE (remove_worker)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::logging());
  options_description.add (gspc::options::scoped_rifd());

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser
      ( boost::unit_test::framework::master_test_suite().argc
      , boost::unit_test::framework::master_test_suite().argv
      )
    . options (options_description).run()
    , vm
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

  test::make const make
    ( installation
    , "remove_worker"
    , test::source_directory (vm)
    , {{"LIB_DESTDIR", installation_dir.string()}}
    , "net lib install"
    );

  gspc::scoped_rifd const rifd { gspc::rifd::strategy (vm)
                               , gspc::rifd::hostnames (vm)
                               , gspc::rifd::port (vm)
                               , installation
                               };

  gspc::scoped_runtime_system drts
    (vm, installation, "worker:1", rifd.entry_points());

  gspc::workflow workflow (make.build_directory() / "remove_worker.pnet");
  workflow.set_wait_for_output();

  boost::asio::io_service io_service;
  std::unique_ptr<fhg::network::connection_type> connection;
  fhg::util::thread::event<void> connected;
  fhg::util::thread::event<void> disconnected;

  fhg::network::continous_acceptor<boost::asio::ip::tcp> acceptor
    ( boost::asio::ip::tcp::endpoint()
    , io_service
    , [] (fhg::network::buffer_type) -> fhg::network::buffer_type
      {
        throw std::logic_error ("Unexpected call to encrypt");
      }
    , [] (fhg::network::buffer_type) -> fhg::network::buffer_type
      {
        throw std::logic_error ("Unexpected call to decrypt");
      }
    , [] ( fhg::network::connection_type*
         , fhg::network::buffer_type
         )
      {
        throw std::logic_error ("Unexpected message");
      }
    , [&connection, &disconnected] (fhg::network::connection_type*)
      {
        connection.reset();
        disconnected.notify();
      }
    , [&connection, &connected]
        (std::unique_ptr<fhg::network::connection_type> c)
      {
        if (!!connection)
        {
          throw std::logic_error ("Unexpected second connection");
        }
        std::swap (connection, c);
        connected.notify();
      }
    );

  const boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
    io_service_thread ([&io_service]() { io_service.run(); });

  struct stop_io_service_on_scope_exit
  {
    ~stop_io_service_on_scope_exit()
    {
      _io_service.stop();
    }
    boost::asio::io_service& _io_service;
  } stop_io_service_on_scope_exit {io_service};

  gspc::job_id_t const job_id
    ( gspc::client (drts).submit
        ( workflow
        , { {"address", fhg::network::connectable_to_address_string
                          (acceptor.local_endpoint().address())
            }
          , {"port", static_cast<unsigned int>
                       (acceptor.local_endpoint().port())
            }
          }
        )
    );

  connected.wait();

  drts.remove_worker (rifd.entry_points());

  disconnected.wait();
}
