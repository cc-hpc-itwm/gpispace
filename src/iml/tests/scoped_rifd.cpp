#include <boost/test/unit_test.hpp>

#include <iml/client/iml.hpp>
#include <iml/client/scoped_rifd.hpp>

#include <boost/program_options.hpp>

#include <iml/testing/parse_command_line.hpp>
#include <iml/testing/scoped_nodefile_from_environment.hpp>
#include <iml/testing/shared_directory.hpp>

#include <util-generic/temporary_path.hpp>

BOOST_AUTO_TEST_CASE (scoped_rifd_from_command_line)
{
  boost::program_options::options_description options_description;

  options_description.add (iml_client::options::installation());
  options_description.add (iml_client::options::scoped_rifd());
  options_description.add (iml_test::options::shared_directory());

  boost::program_options::variables_map vm
    ( iml_test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (iml_test::shared_directory (vm) / "iml_scoped_rifd");

  iml_test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  iml_client::installation const installation (vm);

  iml_client::scoped_rifds const scoped_rifds
    ( iml_client::iml_rifd::strategy {vm}
    , iml_client::iml_rifd::hostnames {vm}
    , iml_client::iml_rifd::port {vm}
    , installation
    );
}
