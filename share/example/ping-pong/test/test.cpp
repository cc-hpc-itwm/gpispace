#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/chrono.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <boost/program_options.hpp>

#include <chrono>

BOOST_AUTO_TEST_CASE (share_example_ping_pong)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
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

  std::string const main ("ping-pong");

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / main);

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path ("install-%%%%-%%%%-%%%%-%%%%"));
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net_lib_install const make
    ( main
    , test::source_directory (vm)
    , installation_dir
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "ping:1 pong:1", rifds.entry_points());

  unsigned long const n (64);

  BOOST_REQUIRE_GT (n, 0u);

  std::chrono::steady_clock::time_point const start
    (std::chrono::steady_clock::now());

  std::multimap<std::string, pnet::type::value::value_type> const result
    (gspc::client (drts).put_and_run
      (gspc::workflow (make.pnet()), {{"n", n}})
    );

  BOOST_REQUIRE_LT
    ( std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::steady_clock::now() - start)
    , std::chrono::milliseconds (3 * n)
    );

  decltype (result) const expected {{"count", n}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
