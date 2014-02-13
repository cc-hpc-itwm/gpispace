#include <stdio.h> // snprintf

#define BOOST_TEST_MODULE GpiSpaceSegmentTest
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include <gpi-space/pc/type/segment_descriptor.hpp>
#include <gpi-space/pc/segment/segment.hpp>

BOOST_AUTO_TEST_CASE ( segment_create_test )
{
  using namespace gpi::pc::segment;
  std::string name ("seg-test-");
  name += boost::lexical_cast<std::string>(getpid());
  segment_t seg (name, 1024);
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
