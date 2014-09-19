// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE share_selftest
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_with_localhost.hpp>
#include <test/scoped_state_directory.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/random_string.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <map>

BOOST_AUTO_TEST_CASE (share_selftest)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::shared_directory());
  options_description.add (test::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser
      ( boost::unit_test::framework::master_test_suite().argc
      , boost::unit_test::framework::master_test_suite().argv
      ).options (options_description).run()
    , vm
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "share_selftest");

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
    , "selftest"
    , test::source_directory (vm)
    , { {"LIB_DESTDIR", installation_dir.string()}
      , {"XML", "selftest.xml"}
      }
    , "net lib install"
    );

    gspc::scoped_runtime_system const drts (vm, installation, "work:1");

    std::string const challenge (fhg::util::random_string_without ("\"\\"));

    std::multimap<std::string, pnet::type::value::value_type> const result
      ( gspc::client (drts)
      . put_and_run ( gspc::workflow (make.build_directory() / "selftest.pnet")
                    , {{"challenge", challenge}}
                    )
      );

    BOOST_REQUIRE_EQUAL (result.size(), 1);

    std::string const port_response ("response");

    BOOST_REQUIRE_EQUAL (result.count (port_response), 1);

    BOOST_CHECK_EQUAL
      ( result.find (port_response)->second
      , pnet::type::value::value_type ("sdpa.response." + challenge)
      );
}
