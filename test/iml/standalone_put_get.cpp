// Copyright (C) 2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/iml/Client.hpp>
#include <gspc/iml/Rifs.hpp>
#include <gspc/iml/RuntimeSystem.hpp>
#include <gspc/iml/SegmentAndAllocation.hpp>
#include <gspc/iml/SharedMemoryAllocation.hpp>

#include <boost/program_options.hpp>

#include <gspc/testing/iml/parse_command_line.hpp>
#include <gspc/testing/iml/set_nodefile_from_environment.hpp>
#include <gspc/testing/iml/virtual_memory_socket_name_for_localhost.hpp>

#include <gspc/util/read_lines.hpp>
#include <gspc/util/split.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/optional.hpp>
#include <gspc/testing/random/string.hpp>

#include <algorithm>
#include <iostream>
#include <string>

namespace
{
  std::size_t const MAX_DATA_LEN = 1024UL * 1024UL;
}

BOOST_AUTO_TEST_CASE (iml_standalone_local_put_get)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::iml::Rifs::options());
  options_description.add (gspc::iml::RuntimeSystem::options());

  ::boost::program_options::variables_map vm
    ( iml_test::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  iml_test::set_nodefile_from_environment (vm);
  iml_test::set_iml_vmem_socket_path_for_localhost (vm);

  vm.notify();

  gspc::iml::Rifs const rifs {vm};
  gspc::iml::RuntimeSystem const iml_rts (rifs, vm, std::cerr);

  gspc::iml::Client client (vm);

  gspc::iml::SegmentAndAllocation const global_data
    ( client
    , gspc::iml::gaspi::SegmentDescription()
    , MAX_DATA_LEN
    );

  std::size_t const offset_metadata = sizeof (std::size_t);

  std::string const data
    (gspc::testing::random_identifier (MAX_DATA_LEN - offset_metadata));

  gspc::iml::SharedMemoryAllocation const write_buffer (client, MAX_DATA_LEN);

  std::size_t const data_size (data.length());
  char* const write_content (write_buffer.pointer());

  std::memcpy (write_content, &data_size, sizeof (data_size));

  std::copy ( data.cbegin()
            , data.cend()
            , write_content + offset_metadata
            );

  client.memcpy
    ( global_data.memory_location()
    , write_buffer.memory_location()
    , data.length() + offset_metadata
    );

  gspc::iml::SharedMemoryAllocation const read_buffer (client, MAX_DATA_LEN);

  auto const metadata_range (global_data.memory_region (0, offset_metadata));
  auto const data_range
    ( global_data.memory_region
        (offset_metadata, MAX_DATA_LEN - offset_metadata)
    );

  client.memcpy
    ( read_buffer.memory_location()
    , metadata_range
    , metadata_range.size
    );

  std::size_t read_size;
  std::memcpy (&read_size, read_buffer.pointer(), sizeof (read_size));

  BOOST_REQUIRE (read_size && read_size <= data_range.size);

  client.memcpy
    ( read_buffer.memory_location (metadata_range.size)
    , data_range
    , read_size
    );

  char const* const read_content (read_buffer.pointer() + metadata_range.size);

  BOOST_REQUIRE_EQUAL (data.length(), read_size);
  BOOST_REQUIRE_EQUAL (data.c_str(), read_content);
}
