// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <iml/vmem/gaspi/pc/memory/beegfs_area.hpp>

#include <iml/MemorySize.hpp>
#include <iml/testing/random/AllocationHandle.hpp>
#include <iml/testing/random/SegmentHandle.hpp>
#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>
#include <iml/vmem/gaspi/pc/segment/segment.hpp>
#include <iml/vmem/gaspi/tests/dummy_topology.hpp>

#include <util-generic/syscall.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>

#include <iml/testing/beegfs_directory.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/test/unit_test.hpp>

#include <fcntl.h>
#include <sys/stat.h>

#include <cstddef>
#include <exception>

struct setup_and_cleanup_shared_file
{
  ::boost::filesystem::path get_beegfs_directory()
  {
    ::boost::program_options::options_description options_description;
    options_description.add (iml_test::options::beegfs_directory());
    ::boost::program_options::variables_map vm;
    ::boost::program_options::store
      ( ::boost::program_options::command_line_parser
          ( ::boost::unit_test::framework::master_test_suite().argc
          , ::boost::unit_test::framework::master_test_suite().argv
          )
      . options (options_description)
      . run()
      , vm
      );
    return iml_test::beegfs_directory (vm);
  }

  setup_and_cleanup_shared_file()
    : path_to_shared_file (get_beegfs_directory() / "beegfs_area_test_area")
  {}

  ::boost::filesystem::path path_to_shared_file;
};

namespace
{
  iml::AllocationHandle random_handle()
  {
    return fhg::util::testing::random<iml::AllocationHandle>{}();
  }
}

BOOST_FIXTURE_TEST_CASE (create_beegfs_segment, setup_and_cleanup_shared_file)
{
  using namespace gpi::pc::memory;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const iml::MemorySize size = 4096;
  const char *text = "hello world!\n";

  {
    beegfs_area_t area ( true
                       , path_to_shared_file
                       , size
                       , topology
                       );

    auto const handle (random_handle());
    auto const segment_id
      (fhg::util::testing::random<iml::SegmentHandle>{}());
    area.alloc (size, gpi::pc::is_global::yes, segment_id, handle);
    area.write_to (iml::MemoryLocation (handle, 0), text, strlen (text));

    {
      int fd = fhg::util::syscall::open
        (((path_to_shared_file / "data").string ().c_str()), O_RDONLY);

      char buf [size];
      std::size_t read_bytes (fhg::util::syscall::read (fd, buf, size));
      BOOST_CHECK_EQUAL (size, iml::MemorySize (read_bytes));
      fhg::util::syscall::close (fd);

      int eq = strncmp (text, buf, strlen (text));
      BOOST_CHECK_EQUAL (0, eq);
    }

    area.free (handle);
  }
}

BOOST_FIXTURE_TEST_CASE (existing_directory_is_failure, setup_and_cleanup_shared_file)
{
  using namespace gpi::pc::memory;

  gpi::tests::dummy_topology topology;

  const iml::MemorySize size = 4096;

  ::boost::filesystem::create_directories (path_to_shared_file);

  BOOST_REQUIRE_THROW  ( gpi::pc::memory::beegfs_area_t ( true
                                                        , path_to_shared_file
                                                        , size
                                                        , topology
                                                        )
                       , std::exception
                       );

  ::boost::filesystem::remove (path_to_shared_file);

  gpi::pc::memory::beegfs_area_t ( true
                                 , path_to_shared_file
                                 , size
                                 , topology
                                 );
}

BOOST_FIXTURE_TEST_CASE (create_big_beegfs_segment, setup_and_cleanup_shared_file)
{
  using namespace gpi::pc::memory;

  gpi::tests::dummy_topology topology;

  const iml::MemorySize size = (1L << 35); // 32 GB

  try
  {
    beegfs_area_t area ( true
                       , path_to_shared_file
                       , size
                       , topology
                       );
  }
  catch (std::exception const& ex)
  {
    BOOST_WARN_MESSAGE ( false
                       , "could not allocate beegfs segment of size: " << size
                       << ": " << ex.what ()
                       << " - please check 'ulimit -v'"
                       );
  }
}

BOOST_FIXTURE_TEST_CASE (create_huge_beegfs_segment, setup_and_cleanup_shared_file)
{
  using namespace gpi::pc::memory;

  gpi::tests::dummy_topology topology;

  const iml::MemorySize size = (1L << 40); // 1 TB

  try
  {
    beegfs_area_t area ( true
                       , path_to_shared_file
                       , size
                       , topology
                       );
  }
  catch (std::exception const& ex)
  {
    BOOST_ERROR ( "could not allocate beegfs segment of size: " << size
                << ": " << ex.what ()
                << " - please check 'ulimit -v'"
                );
  }
}

BOOST_FIXTURE_TEST_CASE (test_read, setup_and_cleanup_shared_file)
{
  using namespace gpi::pc::memory;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const iml::MemorySize size = (1L << 20); // 1 MB
  const char *text = "hello world!\n";

  try
  {
    beegfs_area_t area ( true
                       , path_to_shared_file
                       , size
                       , topology
                       );

    auto const handle (random_handle());
    auto const segment_id
      (fhg::util::testing::random<iml::SegmentHandle>{}());
    area.alloc (size, gpi::pc::is_global::yes, segment_id, handle);

    area.write_to ( iml::MemoryLocation (handle, 0)
                  , text
                  , strlen (text)
                  );

    char buf [size];
    area.read_from ( iml::MemoryLocation (handle, 0)
                   , &buf[0]
                   , size
                   );

    int check = strncmp (text, buf, strlen (text));
    BOOST_CHECK_EQUAL (0, check);

    area.free (handle);
  }
  catch (std::exception const& ex)
  {
    BOOST_ERROR ( "could not allocate beegfs segment of size: " << size
                << ": " << ex.what ()
                << " - please check 'ulimit -v'"
                );
  }
}

BOOST_FIXTURE_TEST_CASE (test_already_open, setup_and_cleanup_shared_file)
{
  using namespace gpi::pc::memory;

  gpi::tests::dummy_topology topology;

  const iml::MemorySize size = (1L << 20); // 1 MB

  beegfs_area_t area ( true
                     , path_to_shared_file
                     , size
                     , topology
                     );

  BOOST_REQUIRE_THROW ( beegfs_area_t ( true
                                      , path_to_shared_file
                                      , size
                                      , topology
                                      )
                      , std::exception
                      );
}
