#define BOOST_TEST_MODULE GpiSpaceSegmentTest
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include <gpi-space/pc/segment/segment.hpp>

BOOST_AUTO_TEST_CASE (sequence_of_create_close_unlink_does_not_throw)
{
  gpi::pc::segment::segment_t seg
    ("seg-test-" + boost::lexical_cast<std::string>(getpid()), 1024);
  try
  {
    seg.create ();
    seg.close ();
    seg.unlink();
  } catch (...)
  {
    seg.unlink();
    throw;
  }
}