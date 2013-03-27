// -*- mode: c++ -*-
#include "base_client.hpp"

#include <fhg/assert.hpp>

#include <boost/foreach.hpp>

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

        m_connection.reset (new connection_type (m_io_service));
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
    }
  }
}
