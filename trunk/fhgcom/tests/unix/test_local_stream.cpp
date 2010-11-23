#define BOOST_TEST_MODULE UnixStream
#include <boost/test/unit_test.hpp>

#include <fhgcom/unix/connection.hpp>

BOOST_AUTO_TEST_CASE (test_server)
{
  using namespace fhg::com::unix;

  typedef server_t<backend<connection_t<message_t> > > my_server;
  my_server unix_stream ("/tmp/foo");

  unix_stream.start ();

  unix_stream.stop();
}
