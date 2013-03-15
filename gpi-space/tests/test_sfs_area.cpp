#define BOOST_TEST_MODULE GpiSpaceSFSAreaTest
#include <boost/test/unit_test.hpp>

#include <sys/types.h> // pid_t
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>    // getpid, unlink
#include <cstring>
#include <fstream>

#include <fhglog/fhglog.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/memory/sfs_area.hpp>
#include <gpi-space/pc/memory/handle_generator.hpp>
#include "dummy_topology.hpp"

namespace fs = boost::filesystem;

static fs::path path_to_shared_file;

struct Setup
{
  Setup ()
  {
    FHGLOG_SETUP ();

    gpi::pc::memory::handle_generator_t::create (42);
  }

  ~Setup ()
  {
    gpi::pc::memory::handle_generator_t::destroy ();
  }
};

BOOST_GLOBAL_FIXTURE (Setup);

struct F
{
  F ()
  {
    path_to_shared_file =
      "/tmp/sfs_area." + boost::lexical_cast<std::string> (getpid ());
    BOOST_TEST_MESSAGE ("fixture setup");
  }

  ~F ()
  {
    gpi::pc::memory::sfs_area_t::cleanup (path_to_shared_file);
    BOOST_TEST_MESSAGE ("fixture teardown");
  }
};

BOOST_FIXTURE_TEST_SUITE( suite, F )

BOOST_AUTO_TEST_CASE (create_sfs_segment)
{
  using namespace gpi::pc::memory;
  using namespace gpi::pc::segment;
  using namespace gpi::pc::global;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = 4096;
  const char *text = "hello world!\n";

  sfs_area_t area ( 0
                  , path_to_shared_file
                  , size
                  , gpi::pc::F_PERSISTENT
                  , topology
                  );
  area.set_id (2);

  BOOST_CHECK_EQUAL (size, area.descriptor().local_size);

  handle_t handle = area.alloc (1, size, "test", 0);

  void *ptr = area.pointer_to (memory_location_t (handle, 0));

  memcpy (ptr, text, strlen (text));

  area.free (handle);

  boost::system::error_code ec;
  area.close (ec);

  int fd = open ( ((path_to_shared_file / "data").string ().c_str ())
                , O_RDONLY
                );
  BOOST_REQUIRE (fd >= 0);

  char buf [size];
  int read_bytes (read (fd, buf, size));
  BOOST_REQUIRE (read_bytes >= 0);
  BOOST_CHECK_EQUAL (size, gpi::pc::type::size_t (read_bytes));
  close (fd);

  int eq = strncmp (text, buf, strlen (text));
  BOOST_CHECK_EQUAL (0, eq);
}

BOOST_AUTO_TEST_CASE (old_segment_version)
{
  using namespace gpi::pc::memory;
  using namespace gpi::pc::segment;
  using namespace gpi::pc::global;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = 4096;

  {
    sfs_area_t area ( 0
                    , path_to_shared_file
                    , size
                    , gpi::pc::F_PERSISTENT
                    , topology
                    );
    area.set_id (2);
  }

  {
    std::ofstream ofs ((path_to_shared_file / "version").string ().c_str ());
    ofs << "SFS version 0" << std::endl;
  }

  try
  {
    sfs_area_t area ( 0
                    , path_to_shared_file
                    , size
                    , gpi::pc::F_PERSISTENT
                    , topology
                    );
    area.set_id (2);
  }
  catch (std::exception const &ex)
  {
    // ok
  }
}

BOOST_AUTO_TEST_CASE (too_new_segment_version)
{
  using namespace gpi::pc::memory;
  using namespace gpi::pc::segment;
  using namespace gpi::pc::global;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = 4096;

  {
    sfs_area_t area ( 0
                    , path_to_shared_file
                    , size
                    , gpi::pc::F_PERSISTENT
                    , topology
                    );
  }

  {
    std::ofstream ofs ((path_to_shared_file / "version").string ().c_str ());
    ofs << "SFS version " << (sfs_area_t::SFS_VERSION + 1) << std::endl;
  }

  try
  {
    sfs_area_t area ( 0
                    , path_to_shared_file
                    , size
                    , gpi::pc::F_PERSISTENT
                    , topology
                    );
    BOOST_CHECK_MESSAGE (false, "succeeded to open a newer sfs area version!");
  }
  catch (std::exception const &ex)
  {
    // ok
  }
}

BOOST_AUTO_TEST_CASE (garbage_segment_version)
{
  using namespace gpi::pc::memory;
  using namespace gpi::pc::segment;
  using namespace gpi::pc::global;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = 4096;

  {
    sfs_area_t area ( 0
                    , path_to_shared_file
                    , size
                    , gpi::pc::F_PERSISTENT
                    , topology
                    );
  }

  {
    std::ofstream ofs ((path_to_shared_file / "version").string ().c_str ());
    ofs << "garbage" << std::endl;
  }

  try
  {
    sfs_area_t area ( 0
                    , path_to_shared_file
                    , size
                    , gpi::pc::F_PERSISTENT
                    , topology
                    );
    BOOST_CHECK_MESSAGE
      (false, "succeeded to open an sfs area with an invalid version!");
  }
  catch (std::exception const &ex)
  {
    // ok
  }
}

BOOST_AUTO_TEST_CASE (reopen_sfs_segment)
{
  using namespace gpi::pc::memory;
  using namespace gpi::pc::segment;
  using namespace gpi::pc::global;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = 4096;
  const char *text = "hello world!\n";

  {
    sfs_area_t area ( 0
                    , path_to_shared_file
                    , size
                    , gpi::pc::F_PERSISTENT
                    , topology
                    );
    area.set_id (2);

    BOOST_CHECK_EQUAL (size, area.descriptor().local_size);

    handle_t handle = area.alloc (1, size, "test", 0);

    void *ptr = area.pointer_to (memory_location_t (handle, 0));

    memcpy (ptr, text, strlen (text));
    area.free (handle);

    boost::system::error_code ec;
    area.close (ec);
  }

  {
    sfs_area_t area ( 0
                    , path_to_shared_file
                    , size
                    , gpi::pc::F_PERSISTENT
                    , topology
                    );
    area.set_id (2);

    BOOST_CHECK_EQUAL (size, area.descriptor().local_size);

    handle_t handle = area.alloc (1, size, "test", 0);

    void *ptr = area.pointer_to (memory_location_t (handle, 0));

    int eq = strncmp (text, (char*)ptr, strlen (text));
    BOOST_CHECK_EQUAL (0, eq);

    area.free (handle);

    boost::system::error_code ec;
    area.close (ec);
  }
}

BOOST_AUTO_TEST_CASE (create_big_sfs_segment)
{
  using namespace gpi::pc::memory;
  using namespace gpi::pc::segment;
  using namespace gpi::pc::global;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = (1L << 35); // 32 GB

  try
  {
    sfs_area_t area ( 0
                    , path_to_shared_file
                    , size
                    , gpi::pc::F_NONE
                    , topology
                    );
    BOOST_CHECK_EQUAL (size, area.descriptor().local_size);
  }
  catch (std::exception const &ex)
  {
    BOOST_WARN_MESSAGE ( false
                       , "could not allocate sfs segment of size: " << size
                       << ": " << ex.what ()
                       << " - please check 'ulimit -v'"
                       );
  }
}

BOOST_AUTO_TEST_CASE (create_huge_sfs_segment_mmap)
{
  using namespace gpi::pc::memory;
  using namespace gpi::pc::segment;
  using namespace gpi::pc::global;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = (1L << 40); // 1 TB

  try
  {
    sfs_area_t area ( 0
                    , path_to_shared_file
                    , size
                    , gpi::pc::F_NONE
                    , topology
                    );
    BOOST_CHECK_EQUAL (size, area.descriptor().local_size);
  }
  catch (std::exception const &ex)
  {
    BOOST_WARN_MESSAGE ( false
                       , "could not allocate sfs segment of size: " << size
                       << ": " << ex.what ()
                       << " - please check 'ulimit -v'"
                       );
  }
}

BOOST_AUTO_TEST_CASE (create_huge_sfs_segment_no_mmap)
{
  using namespace gpi::pc::memory;
  using namespace gpi::pc::segment;
  using namespace gpi::pc::global;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = (1L << 40); // 1 TB

  try
  {
    sfs_area_t area ( 0
                    , path_to_shared_file
                    , size
                    , gpi::pc::F_NOMMAP
                    , topology
                    );
    BOOST_CHECK_EQUAL (size, area.descriptor().local_size);
  }
  catch (std::exception const &ex)
  {
    BOOST_CHECK_MESSAGE ( false
                        , "could not allocate sfs segment of size: " << size
                        << ": " << ex.what ()
                        << " - please check 'ulimit -v'"
                        );
  }
}

BOOST_AUTO_TEST_CASE (test_read)
{
  using namespace gpi::pc::memory;
  using namespace gpi::pc::segment;
  using namespace gpi::pc::global;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = (1L << 20); // 1 MB
  const char *text = "hello world!\n";

  try
  {
    sfs_area_t area ( 0
                    , path_to_shared_file
                    , size
                    , gpi::pc::F_PERSISTENT
                    + gpi::pc::F_NOMMAP
                    , topology
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
    BOOST_CHECK_MESSAGE ( false
                        , "could not allocate sfs segment of size: " << size
                        << ": " << ex.what ()
                        << " - please check 'ulimit -v'"
                        );
  }
}

BOOST_AUTO_TEST_CASE (test_already_open)
{
  using namespace gpi::pc::memory;
  using namespace gpi::pc::segment;
  using namespace gpi::pc::global;
  using namespace gpi::pc::type;

  gpi::tests::dummy_topology topology;

  const gpi::pc::type::size_t size = (1L << 20); // 1 MB

  try
  {
    sfs_area_t area ( 0
                    , path_to_shared_file
                    , size
                    , gpi::pc::F_PERSISTENT
                    + gpi::pc::F_NOMMAP
                    , topology
                    );
    BOOST_CHECK_EQUAL (size, area.descriptor().local_size);
    area.set_id (2);

    sfs_area_t area_51 ( 0
                       , path_to_shared_file
                       , size
                       , gpi::pc::F_PERSISTENT
                       + gpi::pc::F_NOMMAP
                       , topology
                       );

    BOOST_CHECK_MESSAGE ( false
                        , "it was possible to open the same sfs segment twice!"
                        );
  }
  catch (std::exception const &ex)
  {
    // ok
  }
}

BOOST_AUTO_TEST_SUITE_END()
