#define BOOST_TEST_MODULE GpiSpaceSFSAreaTest
#include <boost/test/unit_test.hpp>

#include <sys/types.h> // pid_t
#include <unistd.h>    // getpid, unlink

#include <fhglog/fhglog.hpp>

#include <boost/lexical_cast.hpp>

#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/memory/sfs_area.hpp>
#include "dummy_topology.hpp"

struct SetupLogging
{
  SetupLogging ()
  {
    FHGLOG_SETUP ();
    BOOST_TEST_MESSAGE ("setup logging");
  }

  ~SetupLogging ()
  {}
};

BOOST_GLOBAL_FIXTURE (SetupLogging);

static std::string path_to_shared_file;

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
    unlink (path_to_shared_file.c_str ());
    BOOST_TEST_MESSAGE ("fixture teardown");
  }
};

BOOST_FIXTURE_TEST_SUITE( suite, F )

BOOST_AUTO_TEST_CASE (create_sfs_segment)
{
  using namespace gpi::pc::memory;
  using namespace gpi::pc::segment;
  using namespace gpi::pc::global;

  gpi::tests::dummy_topology topology;

  sfs_area_t area (0, "/tmp/sfs_area.sfs", 4096, 0, "", topology);

  BOOST_CHECK_EQUAL (4096U, area.descriptor().local_size);
}

BOOST_AUTO_TEST_SUITE_END()
