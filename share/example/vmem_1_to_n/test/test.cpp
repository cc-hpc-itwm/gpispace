// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE share_example_vmem_1_to_n
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

#include <we/type/literal/control.hpp>
#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <map>

BOOST_AUTO_TEST_CASE (share_example_vmem_1_to_n)
{
  namespace validators = fhg::util::boost::program_options;

  boost::program_options::options_description options_description;

  constexpr char const* const option_num_bytes ("num-bytes");

  options_description.add_options()
    ( option_num_bytes
    , boost::program_options::value
      <validators::positive_integral<unsigned long>>()->required()
    , "size of bytes per worker"
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
      ).options (options_description).run()
    , vm
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "share_example_vmem_1_to_n");

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
    , "vmem_1_to_n"
    , test::source_directory (vm)
    , {{"LIB_DESTDIR", installation_dir.string()}}
    , "net lib install"
    );

  unsigned long const num_bytes
    (vm.at (option_num_bytes).as<validators::positive_integral<unsigned long>>());

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    ( vm
    , installation
    , "worker:1," + std::to_string (num_bytes)
    , rifds.entry_points()
    );

  gspc::vmem_allocation const allocation_data (drts.alloc (num_bytes, "data"));

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( gspc::client (drts).put_and_run
      ( gspc::workflow (make.build_directory() / "vmem_1_to_n.pnet")
      , { {"memory", allocation_data.global_memory_range()}
        , {"outer", 5L}
        , {"inner", 5L}
        , {"seed", 3141L}
        }
      )
    );

  BOOST_REQUIRE_EQUAL (result.size(), 1);

  std::string const port_out ("out");

  BOOST_REQUIRE_EQUAL (result.count (port_out), 1);

  BOOST_CHECK_EQUAL
    ( result.find (port_out)->second
    , pnet::type::value::value_type (we::type::literal::control())
    );
}
