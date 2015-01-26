// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE share_example_use_sequence
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_with_localhost.hpp>
#include <test/scoped_state_directory.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/temporary_path.hpp>

#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>

#include <map>

namespace
{
  std::multimap<std::string, pnet::type::value::value_type>
    get_result (std::string const& main, long n)
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
      (test::shared_directory (vm) / main);

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
      , main
      , test::source_directory (vm)
      , {{"XML", main + ".xml"}}
      , "net"
      );

    gspc::scoped_runtime_system const drts (vm, installation, "work:4");

    return gspc::client (drts).put_and_run
      (gspc::workflow (make.build_directory() / (main + ".pnet")), {{"n", n}});
  }
}

BOOST_AUTO_TEST_CASE (share_example_use_sequence)
{
  long const n (10);

  std::multimap<std::string, pnet::type::value::value_type> const result
    (get_result ("use_sequence", n));

  BOOST_REQUIRE_EQUAL (result.size(), (n * (n - 1)) / 2);

  std::string const port_out ("out");

  BOOST_REQUIRE_EQUAL (result.count (port_out), result.size());

  std::map<pnet::type::value::value_type, int> number_of_id;

  for ( pnet::type::value::value_type id
      : result.equal_range (port_out) | boost::adaptors::map_values
      )
  {
    ++number_of_id[id];
  }

  for (long id (0); id < n - 1; ++id)
  {
    BOOST_REQUIRE_EQUAL
      (number_of_id.count (pnet::type::value::value_type (id)), 1);
    BOOST_REQUIRE_EQUAL
      (number_of_id.at (pnet::type::value::value_type (id)) + id, n - 1);
  }
}

BOOST_AUTO_TEST_CASE (share_example_use_sequence_bounded)
{
  long const n (10);

  std::multimap<std::string, pnet::type::value::value_type> const result
    (get_result ("use_sequence_bounded", n));

  BOOST_REQUIRE_EQUAL (result.size(), n);

  std::string const port_out ("out");

  BOOST_REQUIRE_EQUAL (result.count (port_out), result.size());

  std::map<pnet::type::value::value_type, int> number_of_id;

  for ( pnet::type::value::value_type id
      : result.equal_range (port_out) | boost::adaptors::map_values
      )
  {
    ++number_of_id[id];
  }

  for (long id (0); id < n - 1; ++id)
  {
    BOOST_REQUIRE_EQUAL
      (number_of_id.count (pnet::type::value::value_type (id)), 1);
    BOOST_REQUIRE_EQUAL
      (number_of_id.at (pnet::type::value::value_type (id)), 1);
  }
}
