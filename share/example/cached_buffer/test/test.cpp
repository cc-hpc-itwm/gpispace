#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>
#include <drts/virtual_memory.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>
#include <test/virtual_memory_socket_name_for_localhost.hpp>

#include <we/type/literal/control.hpp>
#include <we/type/value.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <map>
#include <sstream>

BOOST_AUTO_TEST_CASE (share_example_cached_buffer)
{
  namespace validators = fhg::util::boost::program_options;

  boost::program_options::options_description options_description;

  constexpr char const* const option_vmem_bytes ("vmem-bytes");
  constexpr char const* const option_num_bytes ("num-bytes");
  constexpr char const* const option_cache_bytes ("cache-bytes");
  constexpr char const* const option_num_chunks ("num-chunks");
  constexpr char const* const option_chunk_size ("chunk-size");
  constexpr char const* const option_implementation_lib ("implementation");

  options_description.add_options()
    ( option_vmem_bytes
    , boost::program_options::value
      <validators::positive_integral<unsigned long>>()->required()
    , "size of vmem per worker"
    )
    ;
  options_description.add_options()
    ( option_num_bytes
    , boost::program_options::value
      <validators::positive_integral<long>>()->required()
    , "size of bytes per worker"
    )
    ;
  options_description.add_options()
    ( option_cache_bytes
    , boost::program_options::value
      <validators::positive_integral<long>>()->required()
    , "size of cache per worker"
    )
    ;
  options_description.add_options()
    ( option_num_chunks
    , boost::program_options::value
      <validators::positive_integral<long>>()->required()
    , "number of data chunks"
    )
    ;
  options_description.add_options()
    ( option_chunk_size
    , boost::program_options::value
      <validators::positive_integral<long>>()->required()
    , "number of elements in a chunk"
    )
    ;
  options_description.add_options()
    ( option_implementation_lib
    , boost::program_options::value
      <validators::existing_path>()->required()
    , "path to the library implementing the work function"
    )
    ;
  options_description.add (test::options::shared_directory());
  options_description.add (test::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (gspc::options::virtual_memory());
  options_description.add (gspc::options::logging());

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "share_example_cached_buffer");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);
  test::set_virtual_memory_socket_name_for_localhost (vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net_lib_install const make
    ( installation
    , "cached_buffer"
    , test::source_directory (vm)
    , installation_dir
    , test::option::options()
    . add<test::option::gen::cxx11>()
    . add<test::option::gen::include> (test::source_directory (vm))
    );

  unsigned long const vmem_bytes
    (vm.at (option_vmem_bytes).as<validators::positive_integral<unsigned long>>());
  long const cache_bytes
    (vm.at (option_cache_bytes).as<validators::positive_integral<long>>());
  long const num_bytes
    (vm.at (option_num_bytes).as<validators::positive_integral<long>>());
  long const num_chunks
    (vm.at (option_num_chunks).as<validators::positive_integral<long>>());
  long const chunk_size
    (vm.at (option_chunk_size).as<validators::positive_integral<long>>());
  boost::filesystem::path const implementation_lib
    (vm.at (option_implementation_lib).as<validators::existing_path>());

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  std::ostringstream topology_description;
  topology_description
    << "initialize:1x1,0"
    << " write:" << 1 << "," << std::to_string (num_bytes)
    << " worker:" << 1 << "," << std::to_string (num_bytes) << "," << std::to_string (cache_bytes);

  gspc::scoped_runtime_system const drts
    ( vm
    , installation
    , topology_description.str()
    , rifds.entry_points()
    );

  gspc::vmem_allocation const allocation_data
    ( drts.alloc ( gspc::vmem::gaspi_segment_description()
                 , vmem_bytes
                 , "data"
                 )
    );

  auto const number_of_unique_nodes (rifds.hosts().size());
  long cpu_worker_per_node (1);
  long nwrites (cpu_worker_per_node * number_of_unique_nodes);

  long writechunksize (chunk_size * num_chunks / nwrites);
  long elemsize = sizeof(long);

  if (writechunksize * elemsize > num_bytes)
  {
    writechunksize = num_bytes / elemsize;
    nwrites = 1 + chunk_size * num_chunks / writechunksize;
  }

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( gspc::client (drts).put_and_run
      ( gspc::workflow (make.pnet())
      , { {"memory", allocation_data.global_memory_range()}
        , {"start", we::type::literal::control()}
        , {"num_chunks", num_chunks}
        , {"chunk_size", chunk_size}
        , {"elemsize", elemsize}
        , {"implementation_lib", implementation_lib.string()}
        , {"nwrites", nwrites}
        , {"writechunksize", writechunksize}
        }
      )
    );

  BOOST_REQUIRE_EQUAL (result.size(), 1);
  std::string const port_out ("final_value");
  BOOST_REQUIRE_EQUAL (result.count (port_out), 1);

  BOOST_CHECK_EQUAL
    ( result.find (port_out)->second
    , pnet::type::value::value_type (we::type::literal::control())
    );
}
