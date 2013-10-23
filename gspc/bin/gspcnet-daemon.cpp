#include <iostream>

#include <vector>

#include <boost/foreach.hpp>

#include <gspc/net.hpp>
#include <gspc/net/service/echo.hpp>

int main (int argc, char *argv[])
{
  if (argc == 1)
  {
    std::cerr << "usage: gspcnetd url..." << std::endl;
    return 1;
  }

  gspc::net::initialize ();

  gspc::net::handle ("/service/echo", gspc::net::service::echo ());

  std::vector<gspc::net::server_ptr_t> servers;
  gspc::net::server::queue_manager_t qmgr;

  for (int i = 1 ; i < argc ; ++i)
  {
    try
    {
      gspc::net::server_ptr_t server =
        gspc::net::serve (argv [i], qmgr);

      std::cout << "listening on: "
                << server->url ()
                << std::endl;

      servers.push_back (server);
    }
    catch (std::exception const &ex)
    {
      std::cerr << "could not listen on '" << argv [i] << "': "
                << ex.what ()
                << std::endl;
    }
  }

  if (servers.empty ())
  {
    std::cerr << "nothing to listen on!" << std::endl;
    return 2;
  }

  pause ();

  BOOST_FOREACH (gspc::net::server_ptr_t s, servers)
  {
    s->stop ();
  }

  gspc::net::shutdown ();

  return 0;
}
