#ifndef GSPC_NET_SERVER_HPP
#define GSPC_NET_SERVER_HPP

namespace gspc
{
  namespace net
  {
    /**
       Interface to the generic server.

       gspc::net::server::queue_manager_t qmgr;

       gspc::net::server_t *server = gspc::net::serve
         ("tcp://localhost", qmgr);

       server->start ();

       gspc::net::server_t *server = gspc::net::serve
         ("unix:///tmp/foo", qmgr);

       server->start ();

       acceptor_->async_accept (
     */
    class server_t
    {
    public:
      virtual ~server_t () {}

      virtual int start () = 0;
      virtual int stop () = 0;
    };
  }
}

#endif
