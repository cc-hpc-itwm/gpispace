// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE share_example_n_of_m
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>

#include <we/type/literal/control.hpp>
#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/boost/program_options/validators/executable.hpp>

#include <util-generic/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <map>

BOOST_AUTO_TEST_CASE (share_example_n_of_m)
{
  namespace validators = fhg::util::boost::program_options;

  boost::program_options::options_description options_description;

  constexpr char const* const option_command ("command");

  options_description.add_options()
    ( option_command
    , boost::program_options::value<validators::executable>()->required()
    , "command to execute"
    );

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
    (test::shared_directory (vm) / "share_example_n_of_m");

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
    , test::option::options()
    . add (new test::option::gen::library_path
            (installation.gspc_home() / "libexec" / "gspc")
          )
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "worker:12", rifds.entry_points());

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( gspc::client (drts).put_and_run
      ( gspc::workflow (make.pnet())
      , { {"m", 25L}
        , {"parallel", 2L}
        , {"cmd"
          , boost::filesystem::path
              (vm.at (option_command).as<validators::executable>()).string()
          }
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
}
