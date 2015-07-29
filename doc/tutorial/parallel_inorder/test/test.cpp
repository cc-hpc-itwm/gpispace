// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE tutorial_parallel_inorder
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <map>
#include <vector>

BOOST_AUTO_TEST_CASE (tutorial_parallel_inorder)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
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
    (test::shared_directory (vm) / "tutorial_parallel_inorder");

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
    , "n_of_m"
    , test::source_directory (vm)
    , installation_dir
    , { {"PNETC_OPTS", "-I.."}
      }
    );

  fhg::util::temporary_file const _output_file
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const output_file (_output_file);

  pnet::type::value::value_type config;
  pnet::type::value::poke ("description", config, std::string ("test"));
  pnet::type::value::poke ("output_file", config, output_file.string());

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "work:5", rifds.entry_points());

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( gspc::client (drts).put_and_run
      ( gspc::workflow (make.pnet())
      , { {"n", 5L}
        , {"c", 5L}
        , {"config", config}
        }
      )
    );

  BOOST_REQUIRE_EQUAL (result.size(), 1);

  std::string const port_done ("done");

  BOOST_REQUIRE_EQUAL (result.count (port_done), 1);
  BOOST_CHECK_EQUAL
    ( result.find (port_done)->second
    , pnet::type::value::value_type (we::type::literal::control())
    );

  std::vector<char> const expected_file_content
    { 0, 0, 0
    , 1, 1, 1
    , 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
    , 3, 3, 3, 3, 3, 3
    , 4
    };

  std::string const file_content (fhg::util::read_file (output_file.string()));

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    ( expected_file_content.begin(), expected_file_content.end()
    , file_content.begin(), file_content.end()
    );
}
