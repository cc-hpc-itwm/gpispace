#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <util-generic/first_then.hpp>
#include <util-generic/read_lines.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

#include <chrono>
#include <set>
#include <string>
#include <vector>

std::vector<std::string>  test_nets
  { "net_with_single_module_implementation"
  , "net_with_multiple_module_implementations"
  };

BOOST_AUTO_TEST_CASE (steal_work)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::logging());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add_options()
    ( "test-net"
    , boost::program_options::value<std::string>()->required()
    , "name of the test pnet"
    );

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  auto const net (vm.at ("test-net").as<std::string>());

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / ( "work_stealing"
      + net
      )
    );

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const tmp_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (tmp_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net_lib_install const make
    ( net
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

  gspc::scoped_rifds const rifds
    ( gspc::rifd::strategy {vm}
    , gspc::rifd::hostnames {vm}
    , gspc::rifd::port {vm}
    );

  std::set<std::string> const capabilities {"A", "B", "C"};

  auto const num_workers_of_a_type_per_host
    (fhg::util::testing::random<std::size_t>{} (50, 10));

  std::ostringstream topology;
  fhg::util::first_then<std::string> sep ("", " ");

  for (auto const& capability : capabilities)
  {
    topology << sep << capability << ":" << num_workers_of_a_type_per_host;
  }

  gspc::scoped_runtime_system const drts
    (vm, installation, topology.str(), rifds.entry_points());

  long const num_tasks (6 * num_workers_of_a_type_per_host * hosts.size());
  auto worst_comp_time
    (fhg::util::testing::random<int>{} (10, 3));

  auto start = std::chrono::steady_clock::now();

  std::multimap<std::string, pnet::type::value::value_type> values_on_ports
    { {"num_tasks", num_tasks}
    , {"worst_comp_time", worst_comp_time}
    };

  if (net == test_nets.front())
  {
    values_on_ports.emplace ("slow_worker_host", hosts.front());
  }

  gspc::client (drts)
    .put_and_run (gspc::workflow (make.pnet()), values_on_ports);

  auto end = std::chrono::steady_clock::now();

  BOOST_REQUIRE_GE
    ( std::chrono::duration_cast<std::chrono::seconds> (end - start).count()
    , worst_comp_time
    );

  BOOST_REQUIRE_LE
    ( std::chrono::duration_cast<std::chrono::seconds> (end - start).count()
    , worst_comp_time + 1
    );
}
