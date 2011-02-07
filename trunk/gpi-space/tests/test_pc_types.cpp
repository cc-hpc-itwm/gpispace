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

BOOST_AUTO_TEST_CASE ( segment_descriptor_test )
{
  using namespace gpi::pc::type;

  BOOST_CHECK_EQUAL ( sizeof(segment::list_t)
                    , sizeof(gpi::pc::type::size_t)
                    );

  segment::list_t * list (0);

  char buf1[ sizeof(segment::list_t)
           + 3 * sizeof (segment::list_t::element_type)
           ];
  char buf2[ sizeof(segment::list_t)
           + 3 * sizeof (segment::list_t::element_type)
           ];

  list = ((segment::list_t*)&buf1);
  list->count = 3;
  list->item[0].id = 42;
  list->item[0].perm = 0777;
  list->item[0].ts.created = 0;
  list->item[0].ts.modified = 0;
  list->item[0].ts.accessed = 0;

  list->item[2].id = 1;
  list->item[2].perm = 0777;
  list->item[2].ts.created = 1;
  list->item[2].ts.modified = 2;
  list->item[2].ts.accessed = 3;

  list->item[1].id = 0;
  list->item[1].perm = 0777;
  list->item[1].ts.created = 1;
  list->item[1].ts.modified = 2;
  list->item[1].ts.accessed = 3;

  memcpy (buf2, buf1, sizeof (buf2));

  list = ((segment::list_t*)&buf2);

  BOOST_CHECK_EQUAL (list->count, 3U);
  BOOST_CHECK_EQUAL (list->item[1].id, 0U);
  BOOST_CHECK_EQUAL (list->item[1].ts.modified, 2U);
  BOOST_CHECK_EQUAL (list->item[2].ts.accessed, 3U);
  BOOST_CHECK_EQUAL (segment::traits::is_local  (list->item[2].id), true);
  BOOST_CHECK_EQUAL (segment::traits::is_global (list->item[1].id), true);
  BOOST_CHECK_EQUAL (segment::traits::is_shared (list->item[0].id), true);
}

BOOST_AUTO_TEST_CASE ( handle_descriptor_test )
{
  using namespace gpi::pc::type;

  BOOST_CHECK_EQUAL ( sizeof(handle::list_t)
                    , sizeof(gpi::pc::type::size_t)
                    );

  handle::list_t * list (0);

  char buf[ sizeof(handle::list_t)
          + 3 * sizeof (handle::list_t::element_type)
          ];

  list = ((handle::list_t*)&buf);
  list->count = 3;
  list->item[0].id = 42;
  list->item[0].segment = 0;
  list->item[0].offset = 32;
  list->item[0].size = 128;
  list->item[0].perm = 0777;
  list->item[0].ts.created = 0;
  list->item[0].ts.modified = 0;
  list->item[0].ts.accessed = 0;
}

BOOST_AUTO_TEST_CASE ( proto_alloc_test )
{
  using namespace gpi::pc;
  proto::memory::alloc_t req;
  req.segment = gpi::pc::type::segment::SEG_GLOBAL;
  req.size = 1024;
  req.perm = 0700;

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
    snprintf (req.path, sizeof(req.path), "%s", "/foo");

    proto::segment::attach_reply_t rpl;
    rpl.id = 0;
  }

  {
    proto::segment::detach_t req;
    req.id = 1;

    proto::segment::detach_reply_t rpl;
    rpl.error = 0;
  }

  {
    proto::segment::list_t req;
    (void)req;
    proto::segment::list_reply_t rpl;
    rpl.list.count = 0;
  }
}

BOOST_AUTO_TEST_CASE ( proto_message_test )
{
  using namespace gpi::pc;

  {
    proto::message_t msg;
    msg.length = 1024;
    msg.type = proto::message::memory_alloc;

    msg.type = proto::message::error;

    msg.type = proto::message::segment_detach;
  }
}

BOOST_AUTO_TEST_SUITE_END()
