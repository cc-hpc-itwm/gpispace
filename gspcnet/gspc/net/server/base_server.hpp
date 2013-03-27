#ifndef GSPC_NET_SERVER_BASE_SERVER_HPP
#define GSPC_NET_SERVER_BASE_SERVER_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <gspc/net/server.hpp>
#include <gspc/net/server/base_connection.hpp>
#include <gspc/net/server/queue_manager_fwd.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      template <class Protocol>
      class base_server : public gspc::net::server_t
                        , private boost::noncopyable
      {
      public:
        typedef Protocol                         protocol_type;
        typedef typename protocol_type::endpoint endpoint_type;
        typedef typename protocol_type::acceptor acceptor_type;

        base_server (endpoint_type const &, queue_manager_t &qmgr);
        ~base_server ();

        int start ();
        int stop ();
        std::string url () const;
      private:
        typedef base_connection<protocol_type> connection;
        typedef boost::shared_ptr<connection>  connection_ptr;

        void start_accept ();
        void handle_accept (boost::system::error_code const &);

        queue_manager_t & m_qmgr;

        boost::asio::io_service m_io_service;
        acceptor_type           m_acceptor;
        connection_ptr          m_new_connection;

        typedef boost::shared_ptr<boost::thread> thread_ptr_t;
        typedef std::vector<thread_ptr_t>        thread_pool_t;
        size_t                                   m_thread_pool_size;
        thread_pool_t                            m_thread_pool;
      };
    }
  }
}

#endif
