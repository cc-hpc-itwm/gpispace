// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE share_example_pi
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <map>
#include <sstream>
#include <stdexcept>

BOOST_AUTO_TEST_CASE (share_example_pi)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::shared_directory());
  options_description.add (test::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());

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
    (test::shared_directory (vm) / "share_example_pi");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  test::make const make
    ( installation
    , "pi"
    , test::source_directory (vm)
    , {{"LIB_DESTDIR", installation_dir.string()}}
    , "net lib install"
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "worker:12", rifds.entry_points());

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( gspc::client (drts)
    . put_and_run ( gspc::workflow (make.build_directory() / "pi.pnet")
                  , { {"num_packet", 500L}
                    , {"points_per_packet", 1000000L}
                    , {"credit_generate", 20L}
                    , {"credit_run", 10L}
                    , {"credit_get_key", 20L}
                    , {"seed", 3141L}
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
