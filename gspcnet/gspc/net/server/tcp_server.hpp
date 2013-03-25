#ifndef GSPC_NET_SERVER_TCP_SERVER_HPP
#define GSPC_NET_SERVER_TCP_SERVER_HPP

#include <string>

#include <gspc/net/server.hpp>
#include <gspc/net/server/queue_manager_fwd.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      class tcp_server : public gspc::net::server_t
      {
      public:
        tcp_server ( std::string const &host
                   , std::string const &port
                   , queue_manager_t &qmgr
                   );
        ~tcp_server ();

        int start ();
        int stop ();
      private:
        std::string m_host;
        std::string m_port;
        queue_manager_t & m_qmgr;
      };
    }
  }
}

#endif
