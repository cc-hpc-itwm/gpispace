// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <boost/test/unit_test.hpp>

#include <iml/Client.hpp>
#include <iml/Rifs.hpp>
#include <iml/RuntimeSystem.hpp>
#include <iml/SegmentAndAllocation.hpp>
#include <iml/SharedMemoryAllocation.hpp>

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/optional/optional_io.hpp>

#include <iml/testing/parse_command_line.hpp>
#include <iml/testing/set_nodefile_from_environment.hpp>
#include <iml/testing/virtual_memory_socket_name_for_localhost.hpp>

#include <util-generic/read_lines.hpp>
#include <util-generic/split.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/random/string.hpp>

#include <algorithm>
#include <iostream>
#include <string>

namespace
{
  std::size_t const MAX_DATA_LEN = 1024 * 1024;
}

BOOST_AUTO_TEST_CASE (iml_standalone_local_put_get)
{
  ::boost::program_options::options_description options_description;

  options_description.add (iml::Rifs::options());
  options_description.add (iml::RuntimeSystem::options());

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

  iml::Rifs const rifs {vm};
  iml::RuntimeSystem const iml_rts (rifs, vm, std::cerr);

  iml::Client client (vm);

  iml::SegmentAndAllocation const global_data
    ( client
    , iml::gaspi::SegmentDescription()
    , MAX_DATA_LEN
    );

  std::size_t const offset_metadata = sizeof (std::size_t);

  std::string const data
    (fhg::util::testing::random_identifier (MAX_DATA_LEN - offset_metadata));

  iml::SharedMemoryAllocation const write_buffer (client, MAX_DATA_LEN);

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

  iml::SharedMemoryAllocation const read_buffer (client, MAX_DATA_LEN);

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
