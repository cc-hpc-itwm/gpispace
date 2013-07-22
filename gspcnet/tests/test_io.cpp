#define BOOST_TEST_MODULE GspcNetIo
#include <boost/test/unit_test.hpp>

#include <gspc/net.hpp>
#include <gspc/net/io.hpp>

BOOST_AUTO_TEST_CASE (test_initialize_shutdown)
{
  gspc::net::initialize ();

  {
    gspc::net::server::queue_manager_t qmgr;
    gspc::net::server_ptr_t server =
      gspc::net::serve ("tcp://localhost:*", qmgr);
    BOOST_REQUIRE (server);
  }

  gspc::net::shutdown ();
}
