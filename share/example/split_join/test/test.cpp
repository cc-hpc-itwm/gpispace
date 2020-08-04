#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>

#include <algorithm>
#include <map>
#include <vector>

BOOST_AUTO_TEST_CASE (share_example_split_join)
{
  namespace validators = fhg::util::boost::program_options;

  boost::program_options::options_description options_description;

  constexpr char const* const option_main ("main");
  constexpr char const* const option_input ("input");
  constexpr char const* const option_expected_output ("expected-output");

  options_description.add_options()
    ( option_main
    , boost::program_options::value<validators::nonempty_string>()
      ->required()
    , "name of the main target"
    )
    ( option_input
    , boost::program_options::value<std::vector<long>>()->required()
    , "tokens to put"
    )
    ( option_expected_output
    , boost::program_options::value<std::vector<long>>()->required()
    , "output expected for the given input"
    )
    ;

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

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / boost::filesystem::unique_path ("share_example_split_join-%%%%-%%%%-%%%%-%%%%")
    );

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  std::string const main (vm.at (option_main).as<validators::nonempty_string>());

  test::make_net const make
    ( vm.at (option_main).as<validators::nonempty_string>()
    , test::source_directory (vm)
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 );
  gspc::scoped_runtime_system const drts
    (vm, "work:4", rifds.entry_points());

  std::multimap<std::string, pnet::type::value::value_type> input;

  for (long i : vm.at (option_input).as<std::vector<long>>())
  {
    input.emplace ("I", i);
  }

  std::multimap<std::string, pnet::type::value::value_type> const result
    (gspc::client (drts).put_and_run (gspc::workflow (make.pnet()), input));

  std::multimap<std::string, pnet::type::value::value_type> expected;

  for (long i : vm.at (option_expected_output).as<std::vector<long>>())
  {
    expected.emplace ("O", i);
  }

  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
