// Copyright (C) 2014-2016,2019-2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <map/transform_file/type.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>
#include <gspc/drts/virtual_memory.hpp>

#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/source_directory.hpp>
#include <gspc/testing/virtual_memory_socket_name_for_localhost.hpp>

#include <gspc/we/type/bytearray.hpp>
#include <gspc/we/type/literal/control.hpp>
#include <gspc/we/type/value.hpp>
#include <gspc/testing/printer/we/type/value.hpp>
#include <gspc/we/type/value/read.hpp>

#include <gspc/util/boost/program_options/validators/executable.hpp>
#include <gspc/util/boost/program_options/validators/positive_integral.hpp>
#include <gspc/util/read_file.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <map>
#include <vector>

namespace
{
  std::vector<char> generate_data ( std::filesystem::path const& data_file
                                  , unsigned long size
                                  , unsigned long chunk_size
                                  )
  {
    std::ofstream data (data_file.string(), std::ostream::binary);

    if (!data)
    {
      throw std::runtime_error
        {fmt::format ("Could not open '{}'", data_file.string())};
    }

    std::vector<char> chunk (chunk_size);
    std::vector<char> verify;

    unsigned long bytes_left (size);

    while (bytes_left)
    {
      unsigned long const bytes (std::min (chunk_size, bytes_left));

      for (unsigned long i (0); i < bytes; ++i)
      {
        chunk[i] = gspc::testing::random<char>{}();
      }

      data << std::string (chunk.data(), chunk.data() + bytes);

      std::transform ( chunk.data(), chunk.data() + bytes
                     , std::back_inserter (verify)
                     , [](unsigned char c) { return std::tolower (c); }
                     );

      bytes_left -= bytes;
    }

    return verify;
  }
}

BOOST_AUTO_TEST_CASE (share_example_map_transform_file)
{
  namespace validators = gspc::util::boost::program_options;

  ::boost::program_options::options_description options_description;

  constexpr char const* const option_implementation ("implementation");
  constexpr char const* const option_size_input ("size-input");
  constexpr char const* const option_size_output ("size-output");
  constexpr char const* const option_size_block ("size-block");
  constexpr char const* const option_num_block ("num-block");

  options_description.add_options()
    ( option_implementation
    , ::boost::program_options::value<validators::executable>()->required()
    , "implementation to use"
    )
    ( option_size_input
    , ::boost::program_options::value
      <validators::positive_integral<unsigned long>>()->required()
    , "size for allocation for input data"
    )
    ( option_size_output
    , ::boost::program_options::value
      <validators::positive_integral<unsigned long>>()->required()
    , "size for allocation for output data"
    )
    ( option_size_block
    , ::boost::program_options::value
      <validators::positive_integral<unsigned long>>()->required()
    , "chunk size, data will be processed in chunks of that size"
    )
    ( option_num_block
    , ::boost::program_options::value
      <validators::positive_integral<unsigned long>>()->required()
    , "maximum number of parallel tasks"
    )
    ;

  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  options_description.add (gspc::options::virtual_memory());

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  gspc::util::temporary_path const shared_directory
    (gspc::testing::shared_directory (vm) / "share_example_map_transform_file");

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  gspc::util::temporary_path const _installation_dir
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  auto const installation_dir {std::filesystem::path {_installation_dir}};

  gspc::set_application_search_path (vm, installation_dir);
  gspc::testing::set_virtual_memory_socket_name_for_localhost (vm);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::testing::make_net_lib_install const make
    ( installation
    , "map"
    , gspc::testing::source_directory (vm)
    , installation_dir
    , gspc::testing::option::options()
    . add<gspc::testing::option::gen::include> (gspc::testing::source_directory (vm))
    );

  std::filesystem::path const implementation
    ( std::filesystem::canonical
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

  gspc::util::temporary_file const file_data
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  gspc::util::temporary_file const file_output
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );

  {
    std::ofstream (std::filesystem::path (file_output));
  }

  unsigned long const size (num_block * size_block);

  std::vector<char> const verify (generate_data (file_data, size, 2 << 20));

  gspc::we::type::bytearray const user_data
    ( transform_file::to_bytearray
      (transform_file::parameter (file_data, file_output, size))
    );

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
    ( drts.alloc ( gspc::vmem::gaspi_segment_description()
                 , size_input
                 , "map_input"
                 )
    );
  gspc::vmem_allocation const allocation_output
    ( drts.alloc ( gspc::vmem::gaspi_segment_description()
                 , size_output
                 , "map_output"
                 )
    );

  std::multimap<std::string, gspc::pnet::type::value::value_type> const result
    ( gspc::client (drts).put_and_run
      ( gspc::workflow (make.pnet())
      , { {"input", allocation_input.global_memory_range()}
        , {"output", allocation_output.global_memory_range()}
        , {"num_block", num_block}
        , {"size_block", size_block}
        , {"user_data", user_data}
        , {"implementation", implementation.string()}
        }
      )
    );

  decltype (result) const expected {{"done", gspc::we::type::literal::control()}};
  GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
}
