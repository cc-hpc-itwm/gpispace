// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE doc_tutorial_atomic
#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_with_localhost.hpp>
#include <test/scoped_state_directory.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <boost/program_options.hpp>

#include <map>

BOOST_AUTO_TEST_CASE (doc_tutorial_atomic)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser
      ( boost::unit_test::framework::master_test_suite().argc
      , boost::unit_test::framework::master_test_suite().argv
      ).options (options_description).run()
    , vm
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "doc_tutorial_atomic");

  test::scoped_state_directory const state_directory (shared_directory, vm);
  test::scoped_nodefile_with_localhost const nodefile_with_localhost
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make const make
    ( installation
    , "atomic"
    , test::source_directory (vm)
    , std::unordered_map<std::string, std::string>()
    , "net"
    );

  gspc::scoped_runtime_system const drts (vm, installation, "work:4");

  std::multimap<std::string, pnet::type::value::value_type> const result
    (drts.put_and_run ( make.build_directory() / "atomic.pnet"
                      , { {"number_of_updates", 100UL}
                        , {"initial_value", 12L}
                        }
                      )
    );

  BOOST_REQUIRE_EQUAL (result.size(), 1);

  std::string const port_final_value ("final_value");

  BOOST_REQUIRE_EQUAL (result.count (port_final_value), 1);

  BOOST_CHECK_EQUAL ( result.find (port_final_value)->second
                    , pnet::type::value::value_type (112L)
                    );
}