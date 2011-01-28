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
  }

  ~F()
  {
    gpi::signal::handler().stop();
  }
};

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

    int operator () (gpi::signal::handler_t::signal_t s)
    {
      std::cerr << "got signal " << s << std::endl;
      signal_delivered = s;
      return 0;
    }

    gpi::signal::handler_t::signal_t signal_delivered;
  };
}

BOOST_FIXTURE_TEST_SUITE( suite, F )

BOOST_AUTO_TEST_CASE ( test_connect_disconnect )
{
  dummy_handler hdl;
  gpi::signal::handler_t::connection_t conn
    (gpi::signal::handler().connect (0, boost::ref(hdl)));
  gpi::signal::handler().disconnect (conn);
}

BOOST_AUTO_TEST_CASE ( test_raise )
{
  dummy_handler hdl (-1);
  gpi::signal::handler_t::connection_t conn
    (gpi::signal::handler().connect (10, boost::ref(hdl)));

  BOOST_CHECK_EQUAL (hdl.signal_delivered, -1);

  gpi::signal::handler().raise (10);

  ::raise (SIGINT);

  sleep (10);

  gpi::signal::handler().stop();

  BOOST_CHECK_EQUAL (hdl.signal_delivered, 10);
}

BOOST_AUTO_TEST_SUITE_END()
