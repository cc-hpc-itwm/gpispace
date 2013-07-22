#include "io.hpp"

#include <fhg/assert.hpp>

#include <vector>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace gspc
{
  namespace net
  {
    namespace detail
    {
      struct global_io
      {
        typedef boost::shared_ptr<boost::thread> thread_ptr_t;
        typedef std::vector<thread_ptr_t>        thread_pool_t;
      public:
        global_io ()
          : m_io_service ()
          , m_work ()
          , m_threads ()
        {}

        ~global_io ()
        {
          stop ();
        }

        void start (size_t nthread)
        {
          if (m_threads.empty ())
          {
            m_work.reset (new boost::asio::io_service::work (m_io_service));

            for (size_t i = 0 ; i < nthread ; ++i)
            {
              thread_ptr_t thrd
                (new boost::thread (boost::bind ( &boost::asio::io_service::run
                                                , &m_io_service
                                                )
                                   )
                );
              m_threads.push_back (thrd);
            }
          }
        }

        void stop ()
        {
          if (m_work)
          {
            m_work.reset ();

            BOOST_FOREACH (thread_ptr_t thrd, m_threads)
            {
              thrd->join ();
            }
            m_threads.clear ();
          }
        }

        boost::asio::io_service & service ()
        {
          return m_io_service;
        }
      private:
        boost::asio::io_service                          m_io_service;
        boost::shared_ptr<boost::asio::io_service::work> m_work;
        thread_pool_t                                    m_threads;
      };

      static global_io & get_io_singleton ()
      {
        static global_io gio;
        return gio;
      }
    }

    void initialize ()
    {
      initialize (4);
    }

    void initialize (const size_t nthread)
    {
      detail::get_io_singleton ().start (nthread);
    }
    void shutdown ()
    {
      detail::get_io_singleton ().stop ();
    }

    boost::asio::io_service & io ()
    {
      return detail::get_io_singleton ().service ();
    }
  }
}
