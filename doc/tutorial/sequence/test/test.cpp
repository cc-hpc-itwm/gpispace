// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE doc_tutorial_sequence
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>

#include <list>
#include <map>

BOOST_AUTO_TEST_CASE (doc_tutorial_sequence)
{
  boost::program_options::options_description options_description;

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
    (test::shared_directory (vm) / "doc_tutorial_sequence");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net const make
    ( installation
    , "sequence"
    , test::source_directory (vm)
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "work:1", rifds.entry_points());

  long const n (5);

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( gspc::client (drts).put_and_run
      (gspc::workflow (make.build_directory() / "sequence.pnet"), {{"n", n}})
    );

  BOOST_REQUIRE_EQUAL (result.size(), n);

  std::string const port_i ("i");

  BOOST_REQUIRE_EQUAL (result.count (port_i), n);

  long expected (n);

  for ( pnet::type::value::value_type i
      : result.equal_range (port_i) | boost::adaptors::map_values
      )
  {
    BOOST_REQUIRE_EQUAL (i, pnet::type::value::value_type (--expected));
  }
}
