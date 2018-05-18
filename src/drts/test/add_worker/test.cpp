#include <boost/test/unit_test.hpp>

#include <test_callback.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>
#include <rpc/service_tcp_provider.hpp>

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

#include <we/type/value/boost/test/printer.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <list>
#include <vector>

namespace
{
  gspc::certificates_t const test_certificates
    (boost::filesystem::current_path().parent_path()/"certs");
}

void test_add_worker (gspc::certificates_t const& certificates)
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

  gspc::scoped_runtime_system drts
    (vm, installation, "worker:1", boost::none, master.entry_point(), certificates);

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

  BOOST_REQUIRE_EQUAL (result.size(), 1);

  std::string const port_done ("done");

  BOOST_REQUIRE_EQUAL (result.count (port_done), 1);

  BOOST_CHECK_EQUAL
    ( result.find (port_done)->second
    , pnet::type::value::value_type (we::type::literal::control())
    );
}

BOOST_AUTO_TEST_CASE (add_worker)
{
  test_add_worker (boost::none);
}

BOOST_AUTO_TEST_CASE (add_secure_worker)
{
  test_add_worker (test_certificates);
}

namespace
{
  void store_expected_worker
    ( boost::filesystem::path const& test_dir
    , gspc::scoped_rifds const& rifd
    , std::string const& capability
    , std::set<std::string>& expected_workers
    )
  {
    boost::filesystem::path const ep_file (test_dir / "tmp_entry_point.txt");
    rifd.entry_points().write_to_file (ep_file);

    std::vector<std::string> const entrypoints (fhg::util::read_lines (ep_file));
    BOOST_REQUIRE_EQUAL (entrypoints.size(), 1);

    expected_workers.emplace (capability + "-" + entrypoints.at (0) + "-1");
  }
}

// Note: this test checks the exact identity of the worker executing
// a task that requires a specific capability. However, for the purpose of
// this test it is sufficient to test only if a worker with a given capability
// executed a task requiring that capability. This would make the test
// invariant to subsequent changes in the worker naming convention.
BOOST_AUTO_TEST_CASE (add_workers_with_different_descriptions)
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

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "add_workers_with_different_descriptions");

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
    , "add_workers_with_different_descriptions"
    , test::source_directory (vm)
    , installation_dir
    , test::option::options()
    . add<test::option::gen::include> (test::source_directory (vm))
    . add<test::option::gen::include>
     (test::source_directory (vm).parent_path().parent_path().parent_path())
    . add<test::option::gen::link>
     (vm.at ("rpc-lib").as<boost::filesystem::path>())
    . add<test::option::gen::ld_flag> ("-lboost_coroutine")
    . add<test::option::gen::ld_flag> ("-lboost_context")
    );

  std::vector<std::string> const capabilities {"A", "B", "C"};

  std::list<gspc::scoped_rifds> rifds;

  std::vector<std::string> const hosts
    (fhg::util::read_lines (nodefile_from_environment.path()));

  BOOST_REQUIRE_GT (hosts.size(), 0);

  {
    std::vector<std::string>::const_iterator host (hosts.begin());

    for (unsigned int i (0); i < capabilities.size(); ++i, ++host)
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

  gspc::scoped_runtime_system drts
    (vm, installation, "", boost::none, master.entry_point());

  std::set<std::string> expected_workers;

  unsigned int k {0};
  for (gspc::scoped_rifds const& rifd : rifds)
  {
    store_expected_worker
      (shared_directory, rifd, capabilities[k], expected_workers);

    gspc::worker_description const description
      {{capabilities[k++]}, 1, 0, 0, boost::none};
    drts.add_worker ({description}, rifd.entry_points());
  }

  gspc::client client (drts);

  std::set<std::string> announced_workers;
  fhg::util::scoped_boost_asio_io_service_with_threads io_service (1);
  fhg::rpc::service_dispatcher service_dispatcher;
  fhg::rpc::service_handler<protocol::callback> register_service
    ( service_dispatcher
    , [&announced_workers] (std::string w)
      {
        announced_workers.emplace (w);
      }
    );

  fhg::rpc::service_tcp_provider const registry (io_service, service_dispatcher);

  gspc::job_id_t const job_id
    ( client.submit
        ( gspc::workflow (make.pnet())
        , { {"host", fhg::util::connectable_to_address_string
                       (registry.local_endpoint().address())}
          , {"port", static_cast<unsigned int>
                        (registry.local_endpoint().port())}
          , {"start", true}
          }
        )
    );

  std::multimap<std::string, pnet::type::value::value_type> const result
    (client.wait_and_extract (job_id));

  BOOST_REQUIRE_EQUAL (announced_workers.size(), capabilities.size());

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    ( announced_workers.begin()
    , announced_workers.end()
    , expected_workers.begin()
    , expected_workers.end()
    );

  BOOST_REQUIRE_EQUAL (result.size(), 1);

  std::string const port_done ("all_done");

  BOOST_REQUIRE_EQUAL (result.count (port_done), 1);

  BOOST_CHECK_EQUAL
    ( result.find (port_done)->second
    , pnet::type::value::value_type (we::type::literal::control())
    );
}
