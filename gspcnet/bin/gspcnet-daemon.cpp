#include <iostream>

#include <gspc/net.hpp>

int main (int argc, char *argv[])
{
  gspc::net::server::queue_manager_t qmgr;

  std::string url = argc > 1 ? argv[1] : "tcp://localhost:5000";

  gspc::net::server_ptr_t server =
    gspc::net::serve (url, qmgr);

  pause ();
}
