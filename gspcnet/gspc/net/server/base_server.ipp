// -*- mode: c++ -*-
#include "base_server.hpp"

#include <fhg/assert.hpp>

#include <boost/foreach.hpp>

#include <gspc/net/server/queue_manager.hpp>
#include <gspc/net/server/url_maker.hpp>
#include <gspc/net/frame_builder.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      template <class Proto>
      base_server<Proto>::base_server ( boost::asio::io_service &io
                                      , endpoint_type const &endpoint
                                      , queue_manager_t & qmgr
                                      )
        : m_qmgr (qmgr)
        , m_io_service (io)
        , m_strand (m_io_service)
        , m_endpoint (endpoint)
        , m_acceptor (m_io_service)
        , m_new_connection ()
        , m_queue_length (0)
      {
      }

      template <class Proto>
      base_server<Proto>::~base_server ()
      {
        stop ();
      }

      template <class Proto>
      int base_server<Proto>::start ()
      {
        m_acceptor.open (m_endpoint.protocol());
        m_acceptor.set_option (typename acceptor_type::reuse_address(true));
        m_acceptor.bind (m_endpoint);
        m_acceptor.listen ();

        start_accept();

        return 0;
      }

      template <class Proto>
      int base_server<Proto>::stop ()
      {
        m_acceptor.cancel ();
        m_acceptor.close ();

        if (m_new_connection)
        {
          m_new_connection->stop ();
          m_new_connection.reset ();
        }

        return 0;
      }

      template <class Proto>
      std::string base_server<Proto>::url () const
      {
        return url_maker<Proto>::make (m_acceptor.local_endpoint ());
      }

      template <class Proto>
      int base_server<Proto>::handle_frame (user_ptr user, frame const &f)
      {
        return m_qmgr.handle_frame (user, f);
      }

      template <class Proto>
      int base_server<Proto>::handle_error ( user_ptr user
                                           , boost::system::error_code const &ec
                                           )
      {
        if (ec)
        {
          return m_qmgr.disconnect (user, gspc::net::make::disconnect_frame ());
        }
        return 0;
      }

      template <class Proto>
      void base_server<Proto>::start_accept ()
      {
        m_new_connection.reset (new connection (m_io_service, *this));

        m_acceptor.async_accept ( m_new_connection->socket ()
                                , m_strand.wrap (boost::bind
                                                ( &base_server<Proto>::handle_accept
                                                , this
                                                , boost::asio::placeholders::error
                                                ))
                                );
      }

      template <class Proto>
      void
      base_server<Proto>::handle_accept (boost::system::error_code const &ec)
      {
        if (ec)
        {
          // shutting down
        }
        else
        {
          m_new_connection->set_queue_length (m_queue_length);
          m_new_connection->start ();

          start_accept ();
        }
      }

      template <class Proto>
      void
      base_server<Proto>::set_queue_length (size_t len)
      {
        m_queue_length = len;
      }

      template <class Proto>
      void
      base_server<Proto>::set_thread_pool_size (size_t n)
      {
      }
    }
  }
}
