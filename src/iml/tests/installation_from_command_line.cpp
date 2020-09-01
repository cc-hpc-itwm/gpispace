
#include <boost/test/unit_test.hpp>

#include <iml/client/iml.hpp>

#include <boost/program_options.hpp>

BOOST_AUTO_TEST_CASE (installation_from_command_line)
{
  boost::program_options::options_description options_description;

  options_description.add (iml_client::options::installation());

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser
      ( boost::unit_test::framework::master_test_suite().argc
      , boost::unit_test::framework::master_test_suite().argv
      ).options (options_description).run()
    , vm
    );

  vm.notify();

  iml_client::installation const installation (vm);
}
