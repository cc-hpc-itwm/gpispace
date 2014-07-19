// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE share_example_concurrent
#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>

#include <test/scoped_nodefile_with_localhost.hpp>
#include <test/scoped_state_directory.hpp>
#include <test/setup_logging.hpp>
#include <test/source_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <map>
#include <sstream>
#include <stdexcept>

BOOST_AUTO_TEST_CASE (share_example_concurrent)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (gspc::options::installation());
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
  test::setup_logging (vm);

  vm.notify();

  gspc::installation const installation (vm);

  fhg::util::temporary_path const _build_dir
    ( boost::filesystem::temp_directory_path()
    / boost::filesystem::unique_path()
    );
  boost::filesystem::path const build_dir (_build_dir);

  std::ostringstream command_build;

  command_build
    << " make -f "
    << (installation.gspc_home() / "share" / "sdpa" / "make" / "common.mk")
    << " SDPA_HOME=" << installation.gspc_home()
    << " BOOST_ROOT=" << (installation.gspc_home() / "external" / "boost")
    << " BUILDDIR=" << build_dir
    << " MAIN=concurrent"
    << " -C " << test::source_directory(vm)
    << " net"
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

  gspc::scoped_runtime_system const drts (vm, installation, "");

  std::multimap<std::string, pnet::type::value::value_type> const result
    (drts.put_and_run (build_dir / "concurrent.pnet", {{"N", 1000L}}));

  BOOST_REQUIRE_EQUAL (result.size(), 10);

  std::string const port_call_count_A ("call_count_A");
  std::string const port_call_count_B ("call_count_B");
  std::string const port_call_count_C ("call_count_C");
  std::string const port_call_count_D ("call_count_D");
  std::string const port_sum_call_count ("sum_call_count");

  std::string const port_sum_ids_A ("sum_ids_A");
  std::string const port_sum_ids_B ("sum_ids_B");
  std::string const port_sum_ids_C ("sum_ids_C");
  std::string const port_sum_ids_D ("sum_ids_D");
  std::string const port_sum_ids ("sum_ids");

  BOOST_REQUIRE_EQUAL (result.count (port_call_count_A), 1);
  BOOST_REQUIRE_EQUAL (result.count (port_call_count_B), 1);
  BOOST_REQUIRE_EQUAL (result.count (port_call_count_C), 1);
  BOOST_REQUIRE_EQUAL (result.count (port_call_count_D), 1);
  BOOST_REQUIRE_EQUAL (result.count (port_sum_call_count), 1);
  BOOST_REQUIRE_EQUAL (result.count (port_sum_ids_A), 1);
  BOOST_REQUIRE_EQUAL (result.count (port_sum_ids_B), 1);
  BOOST_REQUIRE_EQUAL (result.count (port_sum_ids_C), 1);
  BOOST_REQUIRE_EQUAL (result.count (port_sum_ids_D), 1);
  BOOST_REQUIRE_EQUAL (result.count (port_sum_ids), 1);

  BOOST_CHECK_EQUAL ( result.find (port_sum_call_count)->second
                    , pnet::type::value::value_type (1000L)
                    );

  auto const in_range
    ([&result] (std::string const& port_call_count) -> void
     {
       long const value
         (boost::get<long> (result.find (port_call_count)->second));

       BOOST_CHECK_GT (value, 200L);
       BOOST_CHECK_LT (value, 300L);
     }
    );

  in_range (port_call_count_A);
  in_range (port_call_count_B);
  in_range (port_call_count_C);
  in_range (port_call_count_D);

  BOOST_CHECK_EQUAL ( result.find (port_sum_ids)->second
                    , pnet::type::value::value_type (499500L)
                    );

  BOOST_CHECK_EQUAL ( boost::get<long> (result.find (port_sum_ids_A)->second)
                    + boost::get<long> (result.find (port_sum_ids_B)->second)
                    + boost::get<long> (result.find (port_sum_ids_C)->second)
                    + boost::get<long> (result.find (port_sum_ids_D)->second)
                    , 499500L
                    );
}
