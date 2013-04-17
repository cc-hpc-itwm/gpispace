// -*- mode: c++ -*-
#include "base_client.hpp"

#include <fhg/assert.hpp>

#include <boost/foreach.hpp>
#include <gspc/net/frame_io.hpp>
#include <gspc/net/frame_builder.hpp>
#include <gspc/net/client/dummy_frame_handler.hpp>

namespace gspc
{
  namespace net
  {
    namespace client
    {
      template <class Proto>
      base_client<Proto>::base_client (endpoint_type const &endpoint)
        : m_io_service ()
        , m_endpoint (endpoint)
        , m_connection ()
        , m_frame_handler (&dummy_frame_handler ())
        , m_thread_pool_size (1)
        , m_thread_pool ()
      {}

      template <class Proto>
      base_client<Proto>::~base_client ()
      {
        stop ();
      }

      template <class Proto>
      int base_client<Proto>::start ()
      {
        assert (m_thread_pool.empty ());

        m_io_service.reset ();

        m_connection.reset (new connection_type (m_io_service, *this));
        m_connection->socket ().connect (m_endpoint);
        m_connection->start ();

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

      template <class Proto>
      int base_client<Proto>::stop ()
      {
        m_io_service.stop ();

        BOOST_FOREACH (thread_ptr_t thrd, m_thread_pool)
        {
          thrd->join ();
        }
        m_thread_pool.clear ();

        return 0;
      }

      template <class Proto>
      void base_client<Proto>::set_frame_handler (frame_handler_t &h)
      {
        m_frame_handler = &h;
      }

      template <class Proto>
      int base_client<Proto>::send_raw (frame const &f)
      {
        return m_connection->deliver (f);
      }

      template <class Proto>
      int base_client<Proto>::send ( std::string const & dst
                                   , std::string const & body
                                   )
      {
        frame f (make::send_frame (header::destination (dst)));
        f.set_body (body);
        f.close ();
        return send_raw (f);
      }

      template <class Proto>
      int base_client<Proto>::request (frame const &f, frame &rply)
      {

        // generate receipt: header
        // add wait object to list of outstanding requests
        // send_raw request
        // wait on wait object
        return -ENOTSUP;
      }

      template <class Proto>
      int base_client<Proto>::subscribe ( std::string const &dst
                                        , std::string const &id
                                        )
      {
        return send_raw
          (make::subscribe_frame ( header::destination (dst)
                                 , header::id (id)
                                 )
          );
      }

      template <class Proto>
      int base_client<Proto>::unsubscribe (std::string const &id)
      {
        return send_raw (make::unsubscribe_frame (header::id (id)));
      }

      template <class Proto>
      int base_client<Proto>::handle_frame (user_ptr user, frame const &f)
      {
        return m_frame_handler->handle_frame (user, f);
      }

      template <class Proto>
      int base_client<Proto>::handle_error ( user_ptr user
                                           , boost::system::error_code const &ec
                                           )
      {
        return m_frame_handler->handle_error (user, ec);
      }
    }
  }
}
