// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_transition_cannot_fire_when_input_port_is_not_connected
#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_with_localhost.hpp>
#include <test/scoped_state_directory.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/temporary_path.hpp>

#include <boost/program_options.hpp>

#include <map>

BOOST_AUTO_TEST_CASE
  (we_transition_cannot_fire_when_input_port_is_not_connected)
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
      )
    . options (options_description).run()
    , vm
    );

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / "we_transition_cannot_fire_when_input_port_is_not_connected"
    );

  test::scoped_state_directory const state_directory (shared_directory, vm);
  test::scoped_nodefile_with_localhost const nodefile_with_localhost
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make const make
    ( installation
    , "transition_with_unconnected_input_port"
    , test::source_directory (vm)
    , std::unordered_map<std::string, std::string> {}
    , "net"
    );

  gspc::scoped_runtime_system const drts (vm, installation, "work:2");

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( drts.put_and_run
      ( make.build_directory() / "transition_with_unconnected_input_port.pnet"
      , {{"i", 0L}}
      )
    );

  BOOST_REQUIRE_EQUAL (result.size(), 0);
}