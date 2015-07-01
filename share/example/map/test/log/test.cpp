// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE share_example_map_log
#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>
#include <drts/virtual_memory.hpp>

#include <test/make.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>
#include <test/virtual_memory_socket_name_for_localhost.hpp>

#include <we/type/bytearray.hpp>
#include <we/type/literal/control.hpp>
#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>
#include <we/type/value/read.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/boost/program_options/validators/executable.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>

#include <util-generic/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <map>

BOOST_AUTO_TEST_CASE (share_example_map_log)
{
  namespace validators = fhg::util::boost::program_options;

  boost::program_options::options_description options_description;

  constexpr char const* const option_implementation ("implementation");
  constexpr char const* const option_size_input ("size-input");
  constexpr char const* const option_size_output ("size-output");
  constexpr char const* const option_size_block ("size-block");
  constexpr char const* const option_num_block ("num-block");
  constexpr char const* const option_user_data ("user-data");

  options_description.add_options()
    ( option_implementation
    , boost::program_options::value<validators::executable>()->required()
    , "implementation to use"
    )
    ( option_size_input
    , boost::program_options::value
      <validators::positive_integral<unsigned long>>()->required()
    , "size for allocation for input data"
    )
    ( option_size_output
    , boost::program_options::value
      <validators::positive_integral<unsigned long>>()->required()
    , "size for allocation for output data"
    )
    ( option_size_block
    , boost::program_options::value
      <validators::positive_integral<unsigned long>>()->required()
    , "chunk size, data will be processed in chunks of that size"
    )
    ( option_num_block
    , boost::program_options::value
      <validators::positive_integral<unsigned long>>()->required()
    , "maximum number of parallel tasks"
    )
    ( option_user_data
    , boost::program_options::value<validators::nonempty_string>()->required()
    , "user data, implementation parameters"
    )
    ;

  options_description.add (test::options::shared_directory());
  options_description.add (test::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (gspc::options::virtual_memory());

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
    (test::shared_directory (vm) / "share_example_map_log");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);
  test::set_virtual_memory_socket_name_for_localhost (vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make const make
    ( installation
    , "map"
    , test::source_directory (vm)
    , { {"LIB_DESTDIR", installation_dir.string()}
      , {"CXXINCLUDEPATHS", test::source_directory (vm).string()}
      , {"PNETC_OPTS", std::string ("--gen-cxxflags=--std=c++11")}
      }
    , "net lib install"
    );

  boost::filesystem::path const implementation
    ( boost::filesystem::canonical
      (vm.at (option_implementation).as<validators::executable>())
    );
  unsigned long const size_input
    (vm.at (option_size_input).as<validators::positive_integral<unsigned long>>());
  unsigned long const size_output
    (vm.at (option_size_output).as<validators::positive_integral<unsigned long>>());
  unsigned long const size_block
    (vm.at (option_size_block).as<validators::positive_integral<unsigned long>>());
  unsigned long const num_block
    (vm.at (option_num_block).as<validators::positive_integral<unsigned long>>());

  std::string const user_data
    (vm.at (option_user_data).as<validators::nonempty_string>());

  std::ostringstream topology_description;

  topology_description << "worker:2," << (2 * size_block);

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, topology_description.str(), rifds.entry_points());

  gspc::vmem_allocation const allocation_input
    (drts.alloc (size_input, "map_input"));
  gspc::vmem_allocation const allocation_output
    (drts.alloc (size_output, "map_output"));

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( gspc::client (drts).put_and_run
      ( gspc::workflow (make.build_directory() / "map.pnet")
      , { {"input", allocation_input.global_memory_range()}
        , {"output", allocation_output.global_memory_range()}
        , {"num_block", num_block}
        , {"size_block", size_block}
        , {"user_data", pnet::type::value::read (user_data)}
        , {"implementation", implementation.string()}
        }
      )
    );

  BOOST_REQUIRE_EQUAL (result.size(), 1);

  std::string const port_done ("done");

  BOOST_REQUIRE_EQUAL (result.count (port_done), 1);

  BOOST_CHECK_EQUAL
    ( result.find (port_done)->second
    , pnet::type::value::value_type (we::type::literal::control())
    );
}
