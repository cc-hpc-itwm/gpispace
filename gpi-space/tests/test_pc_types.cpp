#include <stdio.h> // snprintf

#define BOOST_TEST_MODULE GpiSpacePCTypesTest
#include <boost/test/unit_test.hpp>

#include <fhglog/minimal.hpp>

#include <gpi-space/pc/type/segment_descriptor.hpp>
#include <gpi-space/pc/type/handle_descriptor.hpp>

#include <gpi-space/pc/proto/memory.hpp>
#include <gpi-space/pc/proto/segment.hpp>
#include <gpi-space/pc/proto/message.hpp>

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

BOOST_AUTO_TEST_CASE ( segment_descriptor_test )
{
  using namespace gpi::pc::type;

}

BOOST_AUTO_TEST_CASE ( handle_descriptor_test )
{
  using namespace gpi::pc::type;
}

BOOST_AUTO_TEST_CASE ( proto_alloc_test )
{
  using namespace gpi::pc;
  proto::memory::alloc_t req;
  req.segment = gpi::pc::type::segment::SEG_GPI;
  req.size = 1024;
  req.flags = 0700;

  proto::memory::alloc_reply_t rpl;
  rpl.handle = 0;
}

BOOST_AUTO_TEST_CASE ( proto_free_test )
{
  using namespace gpi::pc;

  proto::memory::free_t req;
  req.handle = 0;
}

BOOST_AUTO_TEST_CASE ( proto_segment_test )
{
  using namespace gpi::pc;

  {
    proto::segment::attach_t req;
    req.id = 1;
  }

  {
    proto::segment::detach_t req;
    req.id = 1;
  }
}

BOOST_AUTO_TEST_CASE ( proto_message_test )
{
  using namespace gpi::pc;
}
