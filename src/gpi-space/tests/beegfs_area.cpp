#define BOOST_TEST_MODULE GpiSpaceBEEGFSAreaTest
#include <boost/test/unit_test.hpp>

#include <sys/types.h> // pid_t
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>    // getpid, unlink
#include <cstring>
#include <fstream>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/memory/beegfs_area.hpp>
#include <gpi-space/pc/memory/handle_generator.hpp>
#include <gpi-space/tests/dummy_topology.hpp>

struct setup_and_cleanup_shared_file
{
  setup_and_cleanup_shared_file ()
  {
    path_to_shared_file =
      "beegfs_area." + boost::lexical_cast<std::string> (getpid ());
  }

  ~setup_and_cleanup_shared_file ()
  {
    gpi::pc::memory::beegfs_area_t::cleanup (path_to_shared_file);
  }

  boost::filesystem::path path_to_shared_file;

  fhg::log::Logger _logger;
};

BOOST_FIXTURE_TEST_CASE (create_beegfs_segment, setup_and_cleanup_shared_file)
{
  gpi::pc::memory::handle_generator_t handle_generator (rand() % (1 << HANDLE_IDENT_BITS));

  using namespace gpi::pc::memory;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = 4096;
  const char *text = "hello world!\n";

  {
    beegfs_area_t area ( _logger
                    , 0
                    , path_to_shared_file
                    , size
                    , gpi::pc::F_PERSISTENT
                    , topology
                    , handle_generator
                    );
    area.set_id (2);

    BOOST_CHECK_EQUAL (size, area.descriptor().local_size);

    handle_t handle = area.alloc (1, size, "test", 0);
    area.write_to (memory_location_t (handle, 0), text, strlen (text));
    area.free (handle);
  }

  int fd = open ( ((path_to_shared_file / "data").string ().c_str ())
                , O_RDONLY
                );
  BOOST_REQUIRE_GE (fd, 0);

  char buf [size];
  int read_bytes (read (fd, buf, size));
  BOOST_REQUIRE_GE (read_bytes, 0);
  BOOST_CHECK_EQUAL (size, gpi::pc::type::size_t (read_bytes));
  close (fd);

  int eq = strncmp (text, buf, strlen (text));
  BOOST_CHECK_EQUAL (0, eq);
}

BOOST_FIXTURE_TEST_CASE (old_segment_version, setup_and_cleanup_shared_file)
{
  gpi::pc::memory::handle_generator_t handle_generator (rand() % (1 << HANDLE_IDENT_BITS));

  using namespace gpi::pc::memory;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = 4096;

  {
    beegfs_area_t area ( _logger
                       , 0
                       , path_to_shared_file
                       , size
                       , gpi::pc::F_PERSISTENT
                       , topology
                       , handle_generator
                       );
    area.set_id (2);
  }

  {
    std::ofstream ofs ((path_to_shared_file / "version").string ().c_str ());
    ofs << "BEEGFS version 0" << std::endl;
  }

  try
  {
    beegfs_area_t area ( _logger
                       , 0
                       , path_to_shared_file
                       , size
                       , gpi::pc::F_PERSISTENT
                       , topology
                       , handle_generator
                       );
    area.set_id (2);
  }
  catch (std::exception const &ex)
  {
    // ok
  }
}

BOOST_FIXTURE_TEST_CASE (too_new_segment_version, setup_and_cleanup_shared_file)
{
  gpi::pc::memory::handle_generator_t handle_generator (rand() % (1 << HANDLE_IDENT_BITS));

  using namespace gpi::pc::memory;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = 4096;

  {
    beegfs_area_t area ( _logger
                       , 0
                       , path_to_shared_file
                       , size
                       , gpi::pc::F_PERSISTENT
                       , topology
                       , handle_generator
                       );
  }

  {
    std::ofstream ofs ((path_to_shared_file / "version").string ().c_str ());
    ofs << "BEEGFS segment version " << (beegfs_area_t::BEEGFS_AREA_VERSION + 1) << std::endl;
  }

  BOOST_REQUIRE_THROW ( beegfs_area_t ( _logger
                                      , 0
                                      , path_to_shared_file
                                      , size
                                      , gpi::pc::F_PERSISTENT
                                      , topology
                                      , handle_generator
                                      )
                      , std::exception
                      );
}

BOOST_FIXTURE_TEST_CASE (garbage_segment_version, setup_and_cleanup_shared_file)
{
  gpi::pc::memory::handle_generator_t handle_generator (rand() % (1 << HANDLE_IDENT_BITS));

  using namespace gpi::pc::memory;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = 4096;

  {
    beegfs_area_t area ( _logger
                       , 0
                       , path_to_shared_file
                       , size
                       , gpi::pc::F_PERSISTENT
                       , topology
                       , handle_generator
                       );
  }

  {
    std::ofstream ofs ((path_to_shared_file / "version").string ().c_str ());
    ofs << "garbage" << std::endl;
  }

  BOOST_REQUIRE_THROW ( beegfs_area_t ( _logger
                                      , 0
                                      , path_to_shared_file
                                      , size
                                      , gpi::pc::F_PERSISTENT
                                      , topology
                                      , handle_generator
                                      )
                      , std::exception
                      );
}

BOOST_FIXTURE_TEST_CASE (reopen_beegfs_segment, setup_and_cleanup_shared_file)
{
  gpi::pc::memory::handle_generator_t handle_generator (rand() % (1 << HANDLE_IDENT_BITS));

  using namespace gpi::pc::memory;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = 4096;
  const char *text = "hello world!\n";

  {
    beegfs_area_t area ( _logger
                       , 0
                       , path_to_shared_file
                       , size
                       , gpi::pc::F_PERSISTENT
                       , topology
                       , handle_generator
                       );
    area.set_id (2);

    BOOST_CHECK_EQUAL (size, area.descriptor().local_size);

    handle_t handle = area.alloc (1, size, "test", 0);
    area.write_to (memory_location_t (handle, 0), text, strlen (text));
    area.free (handle);
  }

  {
    beegfs_area_t area ( _logger
                       , 0
                       , path_to_shared_file
                       , size
                       , gpi::pc::F_PERSISTENT
                       , topology
                       , handle_generator
                       );
    area.set_id (2);

    BOOST_CHECK_EQUAL (size, area.descriptor().local_size);

    handle_t handle = area.alloc (1, size, "test", 0);
    char read_buffer[strlen (text)];
    area.read_from
      (memory_location_t (handle, 0), read_buffer, sizeof (read_buffer));
    BOOST_CHECK_EQUAL (std::string (read_buffer), std::string (text));

    area.free (handle);
  }
}

BOOST_FIXTURE_TEST_CASE (create_big_beegfs_segment, setup_and_cleanup_shared_file)
{
  gpi::pc::memory::handle_generator_t handle_generator (rand() % (1 << HANDLE_IDENT_BITS));

  using namespace gpi::pc::memory;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = (1L << 35); // 32 GB

  try
  {
    beegfs_area_t area ( _logger
                       , 0
                       , path_to_shared_file
                       , size
                       , gpi::pc::F_NONE
                       , topology
                       , handle_generator
                       );
    BOOST_CHECK_EQUAL (size, area.descriptor().local_size);
  }
  catch (std::exception const &ex)
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
  gpi::pc::memory::handle_generator_t handle_generator (rand() % (1 << HANDLE_IDENT_BITS));

  using namespace gpi::pc::memory;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = (1L << 40); // 1 TB

  try
  {
    beegfs_area_t area ( _logger
                       , 0
                       , path_to_shared_file
                       , size
                       , gpi::pc::F_NONE
                       , topology
                       , handle_generator
                       );
    BOOST_CHECK_EQUAL (size, area.descriptor().local_size);
  }
  catch (std::exception const &ex)
  {
    BOOST_ERROR ( "could not allocate beegfs segment of size: " << size
                << ": " << ex.what ()
                << " - please check 'ulimit -v'"
                );
  }
}

BOOST_FIXTURE_TEST_CASE (test_read, setup_and_cleanup_shared_file)
{
  gpi::pc::memory::handle_generator_t handle_generator (rand() % (1 << HANDLE_IDENT_BITS));

  using namespace gpi::pc::memory;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = (1L << 20); // 1 MB
  const char *text = "hello world!\n";

  try
  {
    beegfs_area_t area ( _logger
                       , 0
                       , path_to_shared_file
                       , size
                       , gpi::pc::F_PERSISTENT
                       , topology
                       , handle_generator
                       );
    BOOST_CHECK_EQUAL (size, area.descriptor().local_size);
    area.set_id (2);

    handle_t handle = area.alloc (1, size, "test", 0);

    area.write_to ( memory_location_t (handle, 0)
                  , text
                  , strlen (text)
                  );

    char buf [size];
    area.read_from ( memory_location_t (handle, 0)
                   , &buf[0]
                   , size
                   );

    int check = strncmp (text, buf, strlen (text));
    BOOST_CHECK_EQUAL (0, check);

    area.free (handle);
  }
  catch (std::exception const &ex)
  {
    BOOST_ERROR ( "could not allocate beegfs segment of size: " << size
                << ": " << ex.what ()
                << " - please check 'ulimit -v'"
                );
  }
}

BOOST_FIXTURE_TEST_CASE (test_already_open, setup_and_cleanup_shared_file)
{
  gpi::pc::memory::handle_generator_t handle_generator (rand() % (1 << HANDLE_IDENT_BITS));

  using namespace gpi::pc::memory;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = (1L << 20); // 1 MB

  beegfs_area_t area ( _logger
                     , 0
                     , path_to_shared_file
                     , size
                     , gpi::pc::F_PERSISTENT
                     , topology
                     , handle_generator
                     );
  BOOST_CHECK_EQUAL (size, area.descriptor().local_size);
  area.set_id (2);

  BOOST_REQUIRE_THROW ( beegfs_area_t ( _logger
                                      , 0
                                      , path_to_shared_file
                                      , size
                                      , gpi::pc::F_PERSISTENT
                                      , topology
                                      , handle_generator
                                      )
                      , std::exception
                      );
}
