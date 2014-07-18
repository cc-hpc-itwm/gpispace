// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE share_example_pi
#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>

#include <test/scoped_nodefile_with_localhost.hpp>
#include <test/scoped_state_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/boost/program_options/validators/existing_directory.hpp>

#include <fhg/util/hostname.hpp>
#include <fhg/util/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <map>
#include <sstream>
#include <stdexcept>

BOOST_AUTO_TEST_CASE (share_example_pi)
{
  namespace validators = fhg::util::boost::program_options;

  constexpr char const* const source_directory {"source-directory"};

  boost::program_options::options_description options_description;

  options_description.add_options()
    ( source_directory
    , boost::program_options::value<validators::existing_directory>()
    ->required()
    , "source directory"
    )
    ;

  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::logging());

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser
      ( boost::unit_test::framework::master_test_suite().argc
      , boost::unit_test::framework::master_test_suite().argv
      )
    . options (options_description).run()
    , vm
    );

  test::scoped_state_directory const state_directory (vm);
  test::scoped_nodefile_with_localhost const nodefile_with_localhost (vm);

  //! \todo start logger with specific port
  gspc::set_log_host (vm, fhg::util::hostname());
  gspc::set_log_port (vm, 47095);

  //! \todo either disable gui logging or get the port from a gui manager
  gspc::set_gui_host (vm, fhg::util::hostname());
  gspc::set_gui_port (vm, 47096);

  fhg::util::temporary_path const _installation_dir
    ( boost::filesystem::temp_directory_path()
    / boost::filesystem::unique_path()
    );
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::scoped_runtime_system const drts (vm, " worker:12");

  fhg::util::temporary_path const _build_dir
    ( boost::filesystem::temp_directory_path()
    / boost::filesystem::unique_path()
    );
  boost::filesystem::path const build_dir (_build_dir);

  std::ostringstream command_build;

  command_build
    << " make -f "
    << (drts.gspc_home() / "share" / "sdpa" / "make" / "common.mk")
    << " SDPA_HOME=" << drts.gspc_home()
    << " BOOST_ROOT=" << (drts.gspc_home() / "external" / "boost")
    << " BUILDDIR=" << build_dir
    << " MAIN=pi"
    << " LIB_DESTDIR=" << installation_dir
    << " -C "
    << vm[source_directory].as<validators::existing_directory>()
    << " net lib install"
    ;

  if (int ec = std::system (command_build.str().c_str()) != 0)
  {
    throw std::runtime_error
      (( boost::format ("Could not run '%1%': error code '%1%'")
       % command_build.str()
       % ec
       ).str()
      );
  }

  std::multimap<std::string, pnet::type::value::value_type> const result
    (drts.put_and_run ( build_dir / "pi.pnet"
                      , { {"num_packet", "500L"}
                        , {"points_per_packet", "1000000L"}
                        , {"credit_generate", "20L"}
                        , {"credit_run", "10L"}
                        , {"credit_get_key", "20L"}
                        , {"seed", "3141L"}
                        }
                      )
    );

  BOOST_REQUIRE_EQUAL (result.size(), 1);

  std::string const port_ratio ("ratio");

  BOOST_REQUIRE_EQUAL (result.count (port_ratio), 1);

  pnet::type::value::value_type expected_result;
  pnet::type::value::poke ("in", expected_result, 196347269L);
  pnet::type::value::poke ("total", expected_result, 250000000L);

  BOOST_CHECK_EQUAL (result.find (port_ratio)->second, expected_result);
}
