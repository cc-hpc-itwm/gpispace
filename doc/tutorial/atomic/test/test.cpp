#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <boost/program_options.hpp>

#include <map>

BOOST_AUTO_TEST_CASE (doc_tutorial_atomic)
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

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "doc_tutorial_atomic");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  test::make_net const make
    ( "atomic"
    , test::source_directory (vm)
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 );
  gspc::scoped_runtime_system const drts
    (vm, "work:4", rifds.entry_points());

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( gspc::client (drts). put_and_run ( gspc::workflow (make.pnet())
                                       , { {"number_of_updates", 100UL}
                                         , {"initial_value", 12L}
                                         }
                                       )
    );

  decltype (result) const expected {{"final_value", 112L}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}
