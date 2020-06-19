#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <util-generic/read_lines.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/require_maximum_running_time.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <chrono>
#include <iterator>
#include <set>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE (run_bunch_of_tasks_with_put_token)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add_options()
    ( "num-workers"
    , boost::program_options::value<long>()->required()
    , "number of workers"
    );
  options_description.add_options()
    ( "num-tasks"
    , boost::program_options::value<long>()->required()
    , "number of tasks"
    );

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  long const num_tasks (vm.at ("num-tasks").as<long>());
  long const num_workers (vm.at ("num-workers").as<long>());

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / "run_bunch_of_tasks_with_put_token"
    );

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
    , "run_bunch_of_tasks_with_put_token"
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
    , installation
    );

  gspc::scoped_runtime_system const drts
    ( vm
    , installation
    , "A:" + std::to_string (num_workers)
    , rifds.entry_points()
    );

  gspc::client client (drts);
  pnet::type::value::value_type const start (std::string ("start"));

  FHG_UTIL_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME (std::chrono::seconds (35))
  {
    auto const job_id
      (client.submit
         ( gspc::workflow (make.pnet())
         , {{"start", start}, {"num_tasks", num_tasks}}
         )
      );

    for (unsigned int i {0}; i < num_tasks - 1; i++)
    {
      client.put_token (job_id, "start", start);
    }

    std::multimap<std::string, pnet::type::value::value_type> const result
      (client.wait_and_extract (job_id));

    BOOST_REQUIRE_EQUAL (result.count ("total_tasks_executed"), 1);

    auto const expected (result.equal_range ("total_tasks_executed").first->second);
    BOOST_REQUIRE_EQUAL (boost::get<long>(expected), num_tasks);
  };
}
