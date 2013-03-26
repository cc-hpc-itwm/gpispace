#ifndef GSPC_NET_SERVER_TCP_SERVER_HPP
#define GSPC_NET_SERVER_TCP_SERVER_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <gspc/net/server.hpp>
#include <gspc/net/server/tcp_connection.hpp>
#include <gspc/net/server/queue_manager_fwd.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      class tcp_server : public gspc::net::server_t
                       , private boost::noncopyable
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
        void start_accept ();
        void handle_accept (boost::system::error_code const &);

        std::string m_host;
        std::string m_port;
        queue_manager_t & m_qmgr;

        boost::asio::io_service        m_io_service;
        boost::asio::ip::tcp::acceptor m_acceptor;
        tcp_connection_ptr             m_new_connection;

        typedef boost::shared_ptr<boost::thread> thread_ptr_t;
        typedef std::vector<thread_ptr_t>        thread_pool_t;
        size_t                                   m_thread_pool_size;
        thread_pool_t                            m_thread_pool;
      };
    }
  }
}

#endif
