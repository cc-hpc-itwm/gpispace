#define BOOST_TEST_MODULE drts_add_worker
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
#include <fhg/util/read_lines.hpp>
#include <fhg/util/temporary_path.hpp>
#include <fhg/util/thread/event.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <list>

BOOST_AUTO_TEST_CASE (add_worker)
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
    (test::shared_directory (vm) / "add_worker");

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
    , "add_worker"
    , test::source_directory (vm)
    , {{"LIB_DESTDIR", installation_dir.string()}}
    , "net lib install"
    );

  unsigned int n (3);

  std::list<gspc::scoped_rifd> rifds;

  {
    std::vector<std::string> const hosts
      (fhg::util::read_lines (nodefile_from_environment.path()));

    BOOST_REQUIRE_GT (hosts.size(), 0);

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

  gspc::scoped_runtime_system drts
    (vm, installation, "worker:1", rifds.front().entry_points());

  std::cout << "added" << std::endl;

  gspc::workflow workflow (make.build_directory() / "add_worker.pnet");
  workflow.set_wait_for_output();

  boost::asio::io_service io_service;
  std::list<std::unique_ptr<fhg::network::connection_type>> connections;
  fhg::util::thread::event<void> connected;

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
    , [] (fhg::network::connection_type*) {}
    , [&connections, &connected]
        (std::unique_ptr<fhg::network::connection_type> connection)
      {
        connections.emplace_back (std::move (connection));
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

  gspc::client client (drts);

  gspc::job_id_t const job_id
    ( client.submit
        ( workflow
        , { {"trigger", we::type::literal::control()}
          , {"address", fhg::network::connectable_to_address_string
                          (acceptor.local_endpoint().address())
            }
          , {"port", static_cast<unsigned int>
                       (acceptor.local_endpoint().port())
            }
          , {"wait", n}
          }
        )
    );

  connected.wait();

  for ( std::list<gspc::scoped_rifd>::const_iterator
          rifd (std::next (rifds.begin()))
      ; rifd != rifds.end()
      ; ++rifd
      )
  {
    drts.add_worker (rifd->entry_points());

    client.put_token (job_id, "trigger", we::type::literal::control());

    connected.wait();
  }

  BOOST_REQUIRE_EQUAL (connections.size(), n);

  connections.clear();

  std::multimap<std::string, pnet::type::value::value_type> const result
    (client.wait_and_extract (job_id));

  BOOST_REQUIRE_EQUAL (result.size(), 1);

  std::string const port_done ("done");

  BOOST_REQUIRE_EQUAL (result.count (port_done), 1);

  BOOST_CHECK_EQUAL
    ( result.find (port_done)->second
    , pnet::type::value::value_type (we::type::literal::control())
    );
}
