#include <boost/test/unit_test.hpp>

#include <iml/client/iml.hpp>
#include <iml/client/scoped_rifd.hpp>
#include <iml/client/virtual_memory.hpp>
#include <iml/client/scoped_shm_allocation.hpp>

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/optional/optional_io.hpp>

#include <iml/testing/parse_command_line.hpp>
#include <iml/testing/scoped_nodefile_from_environment.hpp>
#include <iml/testing/shared_directory.hpp>
#include <iml/testing/virtual_memory_socket_name_for_localhost.hpp>

#include <util-generic/read_lines.hpp>
#include <util-generic/split.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/random/string.hpp>

#include <algorithm>
#include <iostream>
#include <string>

namespace
{
  size_t const MAX_DATA_LEN = 1024 * 1024;
}

BOOST_AUTO_TEST_CASE (iml_standalone_local_put_get)
{
  boost::program_options::options_description options_description;

  options_description.add (iml_client::options::installation());
  options_description.add (iml_client::options::scoped_rifd());
  options_description.add (iml_client::options::virtual_memory());
  options_description.add (iml_test::options::shared_directory());

  boost::program_options::variables_map vm
    ( iml_test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (iml_test::shared_directory (vm) / "iml_standalone_local_put_get");

  iml_test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);
  iml_test::set_iml_vmem_socket_path_for_localhost (vm);

  vm.notify();

  iml_client::installation const installation (vm);

  iml_client::scoped_rifds const scoped_rifds
    ( iml_client::iml_rifd::strategy {vm}
    , iml_client::iml_rifd::hostnames {vm}
    , iml_client::iml_rifd::port {vm}
    , installation
    );

  iml_client::scoped_iml_runtime_system const iml_rts
    ( vm
    , installation
    , scoped_rifds.entry_points()
    , std::cerr
    );

  iml_client::vmem_allocation const global_data
    ( iml_rts.alloc ( iml::gaspi_segment_description()
                    , MAX_DATA_LEN
                    , "data"
                    )
    );

  std::size_t const offset_metadata = sizeof (size_t);

  std::string const data
    (fhg::util::testing::random_identifier (MAX_DATA_LEN - offset_metadata));

  iml::client::scoped_shm_allocation const write_buffer
    ( global_data.api()
    , "write_data"
    , MAX_DATA_LEN
    );

  std::size_t const data_size (data.length());
  std::memcpy (global_data.api()->ptr (write_buffer), &data_size, sizeof (data_size));

  char* const write_content
    (static_cast<char*> (global_data.api()->ptr (write_buffer)));
  std::copy ( data.cbegin()
            , data.cend()
            , write_content + offset_metadata
            );

  gpi::pc::type::range_t global_data_range
    (global_data.global_memory_range());

  global_data.api()->memcpy_and_wait
    ( {global_data_range.handle, global_data_range.offset}
    , {write_buffer, 0}
    , data.length() + offset_metadata
    );

  iml::client::scoped_shm_allocation const read_buffer
    ( global_data.api()
    , "read_data"
    , MAX_DATA_LEN
    );

  gpi::pc::type::range_t const metadata_range
    (global_data.global_memory_range (0, offset_metadata));
  gpi::pc::type::range_t const data_range
    (global_data.global_memory_range ( offset_metadata
                                     , MAX_DATA_LEN - offset_metadata
                                     )
    );

  global_data.api()->memcpy_and_wait
    ( {read_buffer, 0}
    , {metadata_range.handle, metadata_range.offset}
    , metadata_range.size
    );

  std::size_t read_size;
  std::memcpy (&read_size, global_data.api()->ptr (read_buffer), sizeof (read_size));

  BOOST_REQUIRE (read_size && read_size <= data_range.size);

  global_data.api()->memcpy_and_wait
    ( {read_buffer, metadata_range.size}
    , {data_range.handle, data_range.offset}
    , read_size
    );

  char* const read_content
    ( static_cast<char*> (global_data.api()->ptr (read_buffer))
    + metadata_range.size
    );

  BOOST_REQUIRE_EQUAL (data.length(), read_size);
  BOOST_REQUIRE_EQUAL (data.c_str(), read_content);
}
