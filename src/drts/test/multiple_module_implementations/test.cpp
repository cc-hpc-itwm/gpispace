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

#include <we/type/value/boost/test/printer.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <iterator>
#include <set>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE (multiple_module_implementations)
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
    / "multiple_module_implementations"
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
    ( "multiple_module_implementations"
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

  std::set<std::string> const preferences {"A", "B", "C"};

  gspc::scoped_runtime_system const drts
    ( vm
    , "A:20 B:20 C:20"
    , rifds.entry_points()
    );

  gspc::client client (drts);

  long const num_tasks (1000);
  std::multimap<std::string, pnet::type::value::value_type> const result
    (client.put_and_run
       ( gspc::workflow (make.pnet())
       , {{"num_tasks", num_tasks}}
       )
    );

  BOOST_REQUIRE_EQUAL (result.count ("done"), 1);

  auto const implementations (result.equal_range ("implementation"));
  BOOST_REQUIRE_EQUAL
    ( std::distance (implementations.first, implementations.second)
    , num_tasks
    );

  for (auto it (implementations.first); it != implementations.second; it++)
  {
    BOOST_REQUIRE (preferences.count (boost::get<std::string>(it->second)));
  }
}
