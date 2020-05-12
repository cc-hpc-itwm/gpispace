// mirko.rahn@itwm.fraunhofer.de

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
#include <we/type/value/poke.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

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

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
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

  test::make_net_lib_install const make
    ( installation
    , "pi"
    , test::source_directory (vm)
    , installation_dir
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "worker:12", rifds.entry_points());

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( gspc::client (drts).put_and_run ( gspc::workflow (make.pnet())
                                      , { {"num_packet", 500L}
                                        , {"points_per_packet", 1000000L}
                                        , {"credit_generate", 20L}
                                        , {"credit_run", 10L}
                                        , {"credit_get_key", 20L}
                                        , {"seed", 3141L}
                                        }
                                      )
    );

  pnet::type::value::value_type expected_ratio;
  pnet::type::value::poke ("in", expected_ratio, 196347269L);
  pnet::type::value::poke ("total", expected_ratio, 250000000L);

  decltype (result) const expected {{"ratio", expected_ratio}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
