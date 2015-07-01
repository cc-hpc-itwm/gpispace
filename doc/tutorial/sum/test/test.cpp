// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE tutorial_sum
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/temporary_path.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>

#include <algorithm>
#include <map>
#include <vector>

namespace
{
  void run_and_check
    ( boost::program_options::variables_map const& vm
    , gspc::installation const& installation
    , boost::filesystem::path const& pnet
    )
  {
    gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                   , gspc::rifd::hostnames {vm}
                                   , gspc::rifd::port {vm}
                                   , installation
                                   );
    gspc::scoped_runtime_system const drts
      (vm, installation, "work:4", rifds.entry_points());

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
      ( gspc::client (drts)
      . put_and_run ( gspc::workflow (pnet)
                    , {{"p", pair (3, 4)}, {"p", pair (-2, 3)}}
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
}

BOOST_AUTO_TEST_CASE (tutorial_sum_expr)
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
    (test::shared_directory (vm) / "tutorial_sum_expr");

  test::scoped_nodefile_from_environment const nodefile_from_environment
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

  run_and_check
    (vm, installation, make.build_directory () / "sum_expr_many.pnet");
}

BOOST_AUTO_TEST_CASE (tutorial_sum_mod)
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
    (test::shared_directory (vm) / "tutorial_sum_mod");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  fhg::util::temporary_path const _sum_module_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const sum_module_dir (_sum_module_dir);

  fhg::util::nest_exceptions<std::runtime_error>
    ([&sum_module_dir, &vm]()
     {
       std::ostringstream make_module;

       make_module
         << "make"
         << " DIR_BUILD=" << sum_module_dir
         << " -C " << (test::source_directory (vm) / "src")
         ;

       fhg::util::system_with_blocked_SIGCHLD (make_module.str());
     }
    , "Could not 'make sum_module'"
    );

  gspc::installation const installation (vm);

  test::make const make
    ( installation
    , "sum_many"
    , test::source_directory (vm)
    , { {"LIB_DESTDIR", installation_dir.string()}
      , {"CXXINCLUDEPATHS", (test::source_directory (vm) / "include").string()}
      , { "PNETC_LINK_PREFIX"
        , (boost::format ("libdir=%1%") % sum_module_dir).str()
        }
      }
    , "net lib install"
    );

  run_and_check (vm, installation, make.build_directory () / "sum_many.pnet");
}
