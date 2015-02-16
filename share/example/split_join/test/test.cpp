// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE share_example_split_join
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/scoped_state_directory.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>

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

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser
      ( boost::unit_test::framework::master_test_suite().argc
      , boost::unit_test::framework::master_test_suite().argv
      ).options (options_description).run()
    , vm
    );

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / boost::filesystem::unique_path ("share_example_split_join-%%%%-%%%%-%%%%-%%%%")
    );

  test::scoped_state_directory const state_directory (shared_directory, vm);
  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  std::string const main (vm.at (option_main).as<validators::nonempty_string>());

  test::make const make
    ( installation
    , vm.at (option_main).as<validators::nonempty_string>()
    , test::source_directory (vm)
    , std::unordered_map<std::string, std::string>()
    , "net"
    );

  gspc::scoped_rifd const rifd ( gspc::rifd::strategy {vm}
                               , gspc::rifd::hostnames {vm}
                               , gspc::rifd::port {vm}
                               , installation
                               );
  gspc::scoped_runtime_system const drts
    (vm, installation, "work:4", rifd.entry_points());

  std::multimap<std::string, pnet::type::value::value_type> input;

  for (long i : vm.at (option_input).as<std::vector<long>>())
  {
    input.emplace ("I", i);
  }

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( gspc::client (drts)
    . put_and_run ( gspc::workflow (make.build_directory() / (main + ".pnet"))
                  , input
                  )
    );

  std::vector<long> const expected_output
    (vm.at (option_expected_output).as<std::vector<long>>());

  BOOST_REQUIRE_EQUAL (result.size(), expected_output.size());

  std::string const port_out ("O");

  BOOST_REQUIRE_EQUAL (result.count (port_out), expected_output.size());

  std::vector<long>::const_iterator expected (expected_output.begin());

  for ( pnet::type::value::value_type i
      : result.equal_range (port_out) | boost::adaptors::map_values
      )
  {
    BOOST_REQUIRE_EQUAL (i, pnet::type::value::value_type (*expected));

    ++expected;
  }
}
