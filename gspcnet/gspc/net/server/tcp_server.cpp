#include "tcp_server.hpp"

#include <fhg/assert.hpp>

#include <boost/foreach.hpp>

#include <gspc/net/server/queue_manager.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      tcp_server::tcp_server ( std::string const & host
                             , std::string const & port
                             , queue_manager_t & qmgr
                             )
        : m_host (host)
        , m_port (port)
        , m_qmgr (qmgr)
        , m_io_service ()
        , m_acceptor (m_io_service)
        , m_new_connection ()
        , m_thread_pool_size (4u)
        , m_thread_pool ()
      {
        // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
        boost::asio::ip::tcp::resolver resolver (m_io_service);
        boost::asio::ip::tcp::resolver::query query (host, port);
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve (query);
        m_acceptor.open (endpoint.protocol());
        m_acceptor.set_option
          (boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind (endpoint);
        m_acceptor.listen ();

        start_accept();
      }

      tcp_server::~tcp_server ()
      {
        stop ();
      }

      int tcp_server::start ()
      {
        assert (m_thread_pool.empty ());

        m_io_service.reset ();

        for (size_t i = 0 ; i < m_thread_pool_size ; ++i)
        {
          thread_ptr_t thrd
            (new boost::thread (boost::bind ( &boost::asio::io_service::run
                                            , &m_io_service
                                            )
                               )
            );
          m_thread_pool.push_back (thrd);
        }

        return 0;
      }

      int tcp_server::stop ()
      {
        m_io_service.stop ();

        BOOST_FOREACH (thread_ptr_t thrd, m_thread_pool)
        {
          thrd->join ();
        }
        m_thread_pool.clear ();

        return 0;
      }

      void tcp_server::start_accept ()
      {
        m_new_connection.reset
          (new tcp_connection ( m_io_service
                              , this
                              , m_qmgr
                              )
          );
        m_acceptor.async_accept ( m_new_connection->socket ()
                                , boost::bind ( &tcp_server::handle_accept
                                              , this
                                              , boost::asio::placeholders::error
                                              )
                                );
      }

      void tcp_server::handle_accept (boost::system::error_code const &ec)
      {
        if (not ec)
        {
          m_new_connection->start ();
        }

        start_accept ();
      }
    }
  }
}
