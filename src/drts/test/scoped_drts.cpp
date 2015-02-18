
#define BOOST_TEST_MODULE drts_scoped_rifd
#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <boost/program_options.hpp>

#include <test/scoped_nodefile_from_environment.hpp>
#include <test/scoped_state_directory.hpp>

#include <test/shared_directory.hpp>

#include <fhg/util/temporary_path.hpp>

BOOST_AUTO_TEST_CASE (scoped_drts_empty_topology)
{
  boost::program_options::options_description options_description;

  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (test::options::shared_directory());

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser
      ( boost::unit_test::framework::master_test_suite().argc
      , boost::unit_test::framework::master_test_suite().argv
      ).options (options_description).run()
    , vm
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "drts_drts");

  test::scoped_state_directory const state_directory (shared_directory, vm);
  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::scoped_rifd const scoped_rifd ( gspc::rifd::strategy {vm}
                                      , gspc::rifd::hostnames {vm}
                                      , gspc::rifd::port {vm}
                                      , installation
                                      );

  gspc::scoped_runtime_system const drts
    (vm, installation, "", scoped_rifd.entry_points());
}
