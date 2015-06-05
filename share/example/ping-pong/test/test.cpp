#define BOOST_TEST_MODULE share_example_ping_pong
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/temporary_path.hpp>

#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>

#include <iomanip>
#include <iostream>
#include <map>

namespace
{
  std::multimap<std::string, pnet::type::value::value_type>
    get_result (std::string const& main, unsigned long const n)
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
        )
      . options (options_description).run()
      , vm
      );

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

    test::make const make
      ( installation
      , main
      , test::source_directory (vm)
      , { {"LIB_DESTDIR", installation_dir.string()}
        }
      , "net lib install"
      );

    gspc::scoped_rifd const rifd ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
    gspc::scoped_runtime_system const drts
      (vm, installation, "ping:1 pong:1", rifd.entry_points());

    return gspc::client (drts).put_and_run
      (gspc::workflow (make.build_directory() / (main + ".pnet")), {{"n", n}});
  }
}

BOOST_AUTO_TEST_CASE (share_example_ping_pong)
{
  unsigned long const n (64);

  BOOST_REQUIRE_GT (n, 0u);

  std::multimap<std::string, pnet::type::value::value_type> const result
    (get_result ("ping-pong", n));

  BOOST_REQUIRE_EQUAL (result.size(), 1u);

  std::string const port_statistics ("statistics");

  BOOST_REQUIRE_EQUAL (result.count (port_statistics), result.size());

  pnet::type::value::value_type const statistics (result.find (port_statistics)->second);
  unsigned long const count
    (boost::get<unsigned long> (*pnet::type::value::peek ("count", statistics)));

  BOOST_REQUIRE_EQUAL (count, n);

  double const min
    (boost::get<double> (*pnet::type::value::peek ("min", statistics)));
  double const max
    (boost::get<double> (*pnet::type::value::peek ("max", statistics)));
  double const sum
    (boost::get<double> (*pnet::type::value::peek ("sum", statistics)));
  double const sqsum
    (boost::get<double> (*pnet::type::value::peek ("sqsum", statistics)));

  double const avg (sum / count);
  double const sdev (std::sqrt ((sqsum / count) - (avg * avg)));

  unsigned long const duration
    ( ( boost::get<unsigned long> (*pnet::type::value::peek ("end", statistics))
      - boost::get<unsigned long> (*pnet::type::value::peek ("start", statistics))
      ) / 1000
    );

  std::cout << count << " pings sent and received, time " << duration << " ms" << std::endl
            << std::fixed << std::setprecision (4)
            << "rtt min/avg/max/sdev " << min << "/" << avg << "/" << max << "/" << sdev << " ms"
            << std::endl
    ;

  BOOST_REQUIRE_LT (avg, 3.0);
}
