// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE tutorial_hello_world
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_with_localhost.hpp>
#include <test/scoped_state_directory.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/temporary_path.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>

#include <map>

BOOST_AUTO_TEST_CASE (tutorial_hello_world)
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
    (test::shared_directory (vm) / "tutorial_hello_world");

  test::scoped_state_directory const state_directory (shared_directory, vm);
  test::scoped_nodefile_with_localhost const nodefile_with_localhost
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  //! \todo allow more than one application search path, use a
  //! separate directory to build the modules...
  // fhg::util::temporary_path const _hello_world_module_dir
  //   (shared_directory / boost::filesystem::unique_path());
  //  boost::filesystem::path const sum_module_dir (_hello_world_module_dir);

  //! \todo ...instead of taking the installation directory
  boost::filesystem::path const sum_module_dir (_installation_dir);

  {
    std::ostringstream make_module;

    make_module
      << "make"
      << " BUILDDIR=" << sum_module_dir
      << " -C " << (test::source_directory (vm) / "src")
      ;

    if ( int ec
       = fhg::util::system_with_blocked_SIGCHLD (make_module.str().c_str())
       )
    {
      throw std::runtime_error
        (( boost::format
           ( "Could not 'make hello_world_module': error code '%1%'"
           ", command was '%2%'"
           )
         % ec
         % make_module.str()
         ).str()
        );
    };
  }

  gspc::installation const installation (vm);

  test::make const make
    ( installation
    , "hello_many"
    , test::source_directory (vm)
    , { {"LIB_DESTDIR", installation_dir.string()}
      , {"CXXINCLUDEPATHS", (test::source_directory (vm) / "include").string()}
      , {"CXXLIBRARYPATHS", sum_module_dir.string()}
      , { "PNETC_LINK_PREFIX"
        , (boost::format ("libdir=%1%") % sum_module_dir).str()
        }
      }
    , "net lib install"
    );

  pnet::type::value::value_type const control {we::type::literal::control()};

  gspc::scoped_runtime_system const drts (vm, installation, "work:4");

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( gspc::client (drts).put_and_run
      ( gspc::workflow (make.build_directory() / "hello_many.pnet")
      , { {"in", control}
        , {"in", control}
        }
      )
    );

  BOOST_REQUIRE_EQUAL (result.size(), 2);

  std::string const port_out ("out");

  BOOST_REQUIRE_EQUAL (result.count (port_out), 2);

  for ( pnet::type::value::value_type i
      : result.equal_range (port_out) | boost::adaptors::map_values
      )
  {
    BOOST_REQUIRE_EQUAL (i, control);
  }
}
