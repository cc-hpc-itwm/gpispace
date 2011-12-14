#define BOOST_TEST_MODULE GpiSpaceSignalHandler
#include <boost/test/unit_test.hpp>

#include <signal.h>

#include <fhglog/minimal.hpp>

#include <gpi-space/signal_handler.hpp>

struct F
{
  F()
  {
    FHGLOG_SETUP();

    gpi::signal::handler().start();

    BOOST_TEST_MESSAGE ("signal handler started");
  }

  ~F()
  {
    gpi::signal::handler().stop();
    BOOST_TEST_MESSAGE ("signal handler stopped");
  }
};

BOOST_GLOBAL_FIXTURE( F );

namespace
{
  struct dummy_handler
  {
    dummy_handler ()
    {}

    template <typename T>
    explicit
    dummy_handler (T t)
      : signal_delivered (t)
    {}

    int store_signal (int s)
    {
      BOOST_TEST_MESSAGE ("got signal " << s);
      signal_delivered = s;
      return 0;
    }

    gpi::signal::handler_t::signal_t signal_delivered;
  };
}

BOOST_AUTO_TEST_CASE ( test_raise )
{
  dummy_handler hdl (-1);
  gpi::signal::handler_t::scoped_connection_t conn
    (gpi::signal::handler().connect( 42
                                   , boost::bind( &dummy_handler::store_signal
                                                , &hdl
                                                , _1
                                                )
                                   )
    );

  BOOST_CHECK_EQUAL (hdl.signal_delivered, -1);

  gpi::signal::handler().raise (42);
  gpi::signal::handler().wait ();

  BOOST_CHECK_EQUAL (hdl.signal_delivered, 42);
}


BOOST_AUTO_TEST_CASE ( test_connect_disconnect )
{
  dummy_handler hdl (-1);
  gpi::signal::handler_t::scoped_connection_t conn
    (gpi::signal::handler().connect( 42
                                   , boost::bind( &dummy_handler::store_signal
                                                , &hdl
                                                , _1
                                                )
                                   )
    );

  BOOST_CHECK_EQUAL (hdl.signal_delivered, -1);

  gpi::signal::handler().raise (42);
  gpi::signal::handler().wait ();

  BOOST_CHECK_EQUAL (hdl.signal_delivered, 42);

  gpi::signal::handler().disconnect (conn);

  hdl.signal_delivered = -1;

  gpi::signal::handler().raise (42);
  gpi::signal::handler().wait ();

  BOOST_CHECK_EQUAL (hdl.signal_delivered, -1);
}
