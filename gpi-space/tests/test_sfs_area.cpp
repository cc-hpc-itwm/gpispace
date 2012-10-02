#define BOOST_TEST_MODULE GpiSpaceSFSAreaTest
#include <boost/test/unit_test.hpp>

#include <sys/types.h> // pid_t
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>    // getpid, unlink
#include <cstring>

#include <fhglog/fhglog.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

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
    //    gpi::pc::memory::sfs_area_t::cleanup (path_to_shared_file);
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
                  , 0
                  , topology
                  );
  area.set_id (2);

  BOOST_CHECK_EQUAL (size, area.descriptor().local_size);

  MLOG (INFO, "allocating " << size << " bytes");

  handle_t handle = area.alloc (1, size, "test", 0);

  MLOG (INFO, "allocated handle := " << handle);

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
  read (fd, buf, size);
  close (fd);

  int eq = strncmp (text, buf, strlen (text));
  BOOST_CHECK_EQUAL (0, eq);

  gpi::pc::memory::sfs_area_t::cleanup (path_to_shared_file);
}

BOOST_AUTO_TEST_SUITE_END()
