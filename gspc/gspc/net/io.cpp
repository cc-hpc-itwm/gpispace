#include "io.hpp"

#include <fhg/assert.hpp>
#include <fhg/util/get_cpucount.h>
#include <fhg/util/threadname.hpp>

#include <vector>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace gspc
{
  namespace net
  {
    namespace
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
          boost::unique_lock<boost::mutex> _ (m_mutex);

          if (not m_io_service)
          {
            m_io_service.reset (new boost::asio::io_service);
            m_work.reset (new boost::asio::io_service::work (*m_io_service));

            for (size_t i = 0 ; i < nthread ; ++i)
            {
              thread_ptr_t thrd
                (new boost::thread (boost::bind ( &global_io::run
                                                , this
                                                , m_io_service
                                                )
                                   )
                );

              fhg::util::set_threadname
                (*thrd, (boost::format ("io-%1%") % i).str ());
              m_threads.push_back (thrd);
            }
          }
        }

        void run (boost::shared_ptr<boost::asio::io_service> my_io)
        {
          assert (my_io);
          my_io->run ();
        }

        void stop ()
        {
          boost::unique_lock<boost::mutex> _ (m_mutex);

          if (m_io_service)
          {
            m_work.reset ();
            m_io_service->stop ();

            BOOST_FOREACH (thread_ptr_t thrd, m_threads)
            {
              thrd->join ();
            }
            m_threads.clear ();

            m_io_service.reset ();
          }
        }

        boost::asio::io_service & service ()
        {
          return *m_io_service;
        }
      private:
        boost::mutex m_mutex;

        boost::shared_ptr<boost::asio::io_service>       m_io_service;
        boost::shared_ptr<boost::asio::io_service::work> m_work;
        thread_pool_t                                    m_threads;
      };

      global_io & get_io_singleton ()
      {
        static global_io gio;
        return gio;
      }

      size_t s_get_thread_count ()
      {
        int ncpu = fhg_get_cpucount ();
        if (ncpu < 0)
          ncpu = 1;
        return ncpu;
      }
    }

    initializer::initializer ()
    {
      get_io_singleton ().start (s_get_thread_count ());
    }

    initializer::initializer (const size_t nthread)
    {
      get_io_singleton ().start (nthread);
    }

    initializer::~initializer ()
    {
      get_io_singleton ().stop ();
    }

    initializer::operator boost::asio::io_service&()
    {
      return get_io_singleton().service();
    }
  }
}
