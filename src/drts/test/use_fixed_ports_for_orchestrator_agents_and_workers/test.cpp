#include <boost/test/unit_test.hpp>

#include <util.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/read_lines.hpp>

#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <vector>

namespace
{
  unsigned int get_free_port()
  {
    boost::asio::io_service service;
    boost::asio::ip::tcp::acceptor acceptor
      (service, boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4(), 0));
    acceptor.set_option (boost::asio::ip::tcp::acceptor::reuse_address (true));

    return acceptor.local_endpoint().port();
  }
}

BOOST_AUTO_TEST_CASE (use_fixed_ports_for_orchestrator_agents_and_workers)
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
    ( test::shared_directory (vm)
    / "use_fixed_ports_for_orchestrator_agents_and_workers"
    );

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  auto const orchestrator_port (get_free_port());
  gspc::set_orchestrator_port (vm, orchestrator_port);

  auto const agent_port (get_free_port());
  gspc::set_agent_port (vm, agent_port);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net_lib_install const make
    ( installation
    , "use_fixed_ports_for_orchestrator_agents_and_workers"
    , test::source_directory (vm)
    , installation_dir
    , test::option::options()
    . add<test::option::gen::include> (test::source_directory (vm))
    . add<test::option::gen::include>
        (test::source_directory (vm).parent_path().parent_path().parent_path())
    );

  std::vector<std::string> const hosts
    (fhg::util::read_lines (nodefile_from_environment.path()));

  BOOST_REQUIRE_GT (hosts.size(), 0);

  gspc::scoped_rifds rifds
    ( gspc::rifd::strategy (vm)
    , gspc::rifd::hostnames ({hosts.front()})
    , gspc::rifd::port (vm)
    , installation
    );

  auto const worker_port (get_free_port());

  gspc::scoped_runtime_system drts
    ( vm
    , installation
    , "worker:1/" + std::to_string (worker_port)
    , rifds.entry_points()
    );

  BOOST_REQUIRE (is_using_port ("orchestrator", orchestrator_port));
  BOOST_REQUIRE (is_using_port ("agent", agent_port));

  gspc::client client (drts);

  gspc::job_id_t const job_id
    ( client.submit
        ( gspc::workflow (make.pnet())
        , { {"port", worker_port}
          , {"start", true}
          }
        )
    );

  std::multimap<std::string, pnet::type::value::value_type> const result
    (client.wait_and_extract (job_id));

  BOOST_REQUIRE_EQUAL (result.size(), 1);
  BOOST_REQUIRE_EQUAL (result.count ("port_is_used"), 1);

  BOOST_REQUIRE_EQUAL
    ( result.find ("port_is_used")->second
    , pnet::type::value::value_type (true)
    );
}
