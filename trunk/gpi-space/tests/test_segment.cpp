#include <stdio.h> // snprintf

#define BOOST_TEST_MODULE GpiSpaceSegmentTest
#include <boost/test/unit_test.hpp>

#include <fhglog/minimal.hpp>

#include <gpi-space/pc/type/segment_descriptor.hpp>
#include <gpi-space/pc/segment/segment.hpp>

struct SetupLogging
{
  SetupLogging()
  {
    FHGLOG_SETUP();
    BOOST_TEST_MESSAGE ("setup logging");
  }

  ~SetupLogging()
  {}
};

BOOST_GLOBAL_FIXTURE( SetupLogging );

struct F
{
  F()
  {
    BOOST_TEST_MESSAGE ("fixture setup");
  }

  ~F ()
  {
    BOOST_TEST_MESSAGE ("fixture teardown");
  }
};

BOOST_FIXTURE_TEST_SUITE( suite, F )

BOOST_AUTO_TEST_CASE ( segment_create_test )
{
  using namespace gpi::pc::segment;
  segment_t seg ("foo", 1024, gpi::pc::type::segment::SEG_INVAL);
  seg.create ();
}

BOOST_AUTO_TEST_SUITE_END()
