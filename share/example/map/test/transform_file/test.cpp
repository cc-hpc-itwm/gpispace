// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE share_example_map_transform_file
#include <boost/test/unit_test.hpp>

#include <map/test/transform_file/type.hpp>

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
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/read_file.hpp>
#include <fhg/util/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <algorithm>
#include <fstream>
#include <map>
#include <random>
#include <vector>

namespace
{
  std::vector<char> generate_data ( boost::filesystem::path const& data_file
                                  , unsigned long size
                                  , unsigned long chunk_size
                                  )
  {
    std::ofstream data (data_file.string(), std::ostream::binary);

    if (!data)
    {
      std::runtime_error
        ((boost::format ("Could not open '%1%'") % data_file).str());
    }

    std::mt19937 generator;
    std::uniform_int_distribution<> number (0, 255);

    std::vector<char> chunk (chunk_size);
    std::vector<char> verify;

    unsigned long bytes_left (size);

    while (bytes_left)
    {
      unsigned long const bytes (std::min (chunk_size, bytes_left));

      for (unsigned long i (0); i < bytes; ++i)
      {
        chunk[i] = number (generator);
      }

      data << std::string (chunk.data(), chunk.data() + bytes);

      std::transform ( chunk.data(), chunk.data() + bytes
                     , std::back_inserter (verify)
                     , [](char c) { return std::tolower (c); }
                     );

      bytes_left -= bytes;
    }

    return verify;
  }
}

BOOST_AUTO_TEST_CASE (share_example_map_transform_file)
{
  namespace validators = fhg::util::boost::program_options;

  boost::program_options::options_description options_description;

  constexpr char const* const option_implementation ("implementation");
  constexpr char const* const option_size_input ("size-input");
  constexpr char const* const option_size_output ("size-output");
  constexpr char const* const option_size_block ("size-block");
  constexpr char const* const option_num_block ("num-block");

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
    (test::shared_directory (vm) / "share_example_map_transform_file");

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

  fhg::util::temporary_file const file_data
    (shared_directory / boost::filesystem::unique_path());
  fhg::util::temporary_file const file_output
    (shared_directory / boost::filesystem::unique_path());

  {
    std::ofstream (boost::filesystem::path (file_output).string());
  }

  unsigned long const size (num_block * size_block);

  std::vector<char> const verify (generate_data (file_data, size, 2 << 20));

  we::type::bytearray const user_data
    ( transform_file::to_bytearray
      (transform_file::parameter (file_data, file_output, size))
    );

  std::ostringstream topology_description;

  topology_description << "worker:2," << (2 * size_block);

  gspc::scoped_rifd const rifd ( gspc::rifd::strategy {vm}
                               , gspc::rifd::hostnames {vm}
                               , gspc::rifd::port {vm}
                               , installation
                               );
  gspc::scoped_runtime_system const drts
    (vm, installation, topology_description.str(), rifd.entry_points());

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
        , {"user_data", user_data}
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

  std::string const output (fhg::util::read_file (file_output));

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    (verify.begin(), verify.end(), output.begin(), output.end());
}
