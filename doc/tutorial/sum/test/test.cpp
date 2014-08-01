// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE tutorial_sum
#include <boost/test/unit_test.hpp>

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

#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>

#include <algorithm>
#include <map>
#include <vector>

BOOST_AUTO_TEST_CASE (tutorial_sum_expr)
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
    (test::shared_directory (vm) / "tutorial_sum_expr");

  test::scoped_state_directory const state_directory (shared_directory, vm);
  test::scoped_nodefile_with_localhost const nodefile_with_localhost
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make const make
    ( installation
    , "sum_expr_many"
    , test::source_directory (vm)
    , std::unordered_map<std::string, std::string> {}
    , "net"
    );

  gspc::scoped_runtime_system const drts (vm, installation, " work:4");

  auto pair
    ( [] (long x, long y) -> pnet::type::value::value_type
      {
        pnet::type::value::value_type v;
        pnet::type::value::poke ("x", v, x);
        pnet::type::value::poke ("y", v, y);
        return v;
      }
    );

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( drts.put_and_run
      ( make.build_directory() / "sum_expr_many.pnet"
      , { {"p", pair (3, 4)}
        , {"p", pair (-2, 3)}
        }
      )
    );

  BOOST_REQUIRE_EQUAL (result.size(), 2);

  std::string const port_s ("s");

  BOOST_REQUIRE_EQUAL (result.count (port_s), 2);

  std::vector<long> const expected_output {1L, 7L};
  std::vector<long> output;

  for ( pnet::type::value::value_type i
      : result.equal_range (port_s) | boost::adaptors::map_values
      )
  {
    output.push_back (boost::get<long> (i));
  }

  std::sort (output.begin(), output.end());

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    ( expected_output.begin(), expected_output.end()
    , output.begin(), output.end()
    );
}
