// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE share_lib_cache_demo
#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_with_localhost.hpp>
#include <test/scoped_state_directory.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/temporary_path.hpp>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>

#include <map>

BOOST_AUTO_TEST_CASE (share_lib_cache_demo)
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
    (test::shared_directory (vm) / "share_lib_cache_demo");

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
    , "demo"
    , test::source_directory (vm)
    , { {"LIB_DESTDIR", installation_dir.string()}
      , {"PNETC_OPTS", ( boost::format ("-I%1%")
                       % (installation.gspc_home() / "share" / "sdpa" / "xml")
                       ).str()
        }
      }
    , "net lib install"
    );

  gspc::scoped_runtime_system const drts (vm, installation, "work:4");

  long const num_id (6);
  long const multiplicity (4);

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( drts.put_and_run
      ( make.build_directory() / "demo.pnet"
      , { {"num_slots", 4L}
        , {"num_id", num_id}
        , {"multiplicity", multiplicity}
        }
      )
    );

  BOOST_REQUIRE_EQUAL (result.size(), num_id * multiplicity);

  std::string const port_id ("id");

  BOOST_REQUIRE_EQUAL (result.count (port_id), num_id * multiplicity);

  std::map<pnet::type::value::value_type, int> number_of_id_in_output;

  for ( pnet::type::value::value_type id
      : result.equal_range (port_id) | boost::adaptors::map_values
      )
  {
    ++number_of_id_in_output[id];
  }

  BOOST_REQUIRE_EQUAL (number_of_id_in_output.size(), num_id);

  for (long id (0); id < num_id; ++id)
  {
    BOOST_REQUIRE_EQUAL
      (number_of_id_in_output.count (pnet::type::value::value_type (id)), 1);
    BOOST_REQUIRE_EQUAL
      ( number_of_id_in_output.at (pnet::type::value::value_type (id))
      , multiplicity
      );
  }
}