// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE share_example_n_of_m
#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_with_localhost.hpp>
#include <test/scoped_state_directory.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>

#include <we/type/literal/control.hpp>
#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/boost/program_options/validators/executable.hpp>

#include <fhg/util/temporary_path.hpp>

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
    (test::shared_directory (vm) / "share_example_n_of_m");

  test::scoped_state_directory const state_directory (shared_directory, vm);
  test::scoped_nodefile_with_localhost const nodefile_with_localhost
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  test::make const make
    ( installation
    , "n_of_m"
    , test::source_directory (vm)
    , { { "LIB_DESTDIR", installation_dir.string()}
      , { "CXXLIBRARYPATHS"
        , (installation.gspc_home() / "libexec" / "sdpa").string()
        }
      }
    , "net lib install"
    );

  gspc::scoped_runtime_system const drts (vm, installation, "worker:12");

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( drts.put_and_run
      ( make.build_directory() / "n_of_m.pnet"
      , { {"m", 25L}
        , {"parallel", 2L}
        , {"cmd"
          , boost::filesystem::path
              (vm[option_command].as<validators::executable>()).string()
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