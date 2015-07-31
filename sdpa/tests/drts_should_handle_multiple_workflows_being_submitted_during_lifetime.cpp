// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE sdpa_test_drts_should_handle_multiple_workflows_being_submitted_during_lifetime
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random_string.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <map>

BOOST_AUTO_TEST_CASE (sdpa_test_drts_should_handle_multiple_workflows_being_submitted_during_lifetime)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::shared_directory());
  options_description.add (test::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
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
    / "sdpa_test_drts_should_handle_multiple_workflows_being_submitted_during_lifetime"
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
    , "selftest"
    , test::source_directory (vm)
    , installation_dir
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "work:1", rifds.entry_points());

    std::string const challenge (fhg::util::testing::random_string_without ("\"\\"));

    for (int i (0); i < 2; ++i)
    {
      std::multimap<std::string, pnet::type::value::value_type> const result
        ( gspc::client (drts).put_and_run
            (gspc::workflow (make.pnet()), {{"challenge", challenge}})
        );

      BOOST_REQUIRE_EQUAL (result.size(), 1);

      std::string const port_response ("response");

      BOOST_REQUIRE_EQUAL (result.count (port_response), 1);

      BOOST_CHECK_EQUAL
        ( result.find (port_response)->second
        , pnet::type::value::value_type ("sdpa.response." + challenge)
        );
    }
}
