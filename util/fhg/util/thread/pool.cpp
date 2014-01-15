#include "pool.hpp"

#include <stdexcept>
#include <boost/format.hpp>

#include <fhg/util/threadname.hpp>
#include <fhg/util/get_cpucount.h>

namespace fhg
{
  namespace thread
  {
    pool_t::pool_t (std::size_t nthread, std::string const &name)
      : m_stop (false)
      , m_nthread ( nthread == 0
                  ? throw std::invalid_argument
                    ("fhg::thread::pool_t: nthreads needs to be > 0")
                  : nthread
                  )
      , m_pool_name (name)
    {
      for (std::size_t i = 0 ; i != m_nthread ; ++i)
      {
        m_threads.push_back
          (new boost::thread (&pool_t::worker, this, i));
        fhg::util::set_threadname
          ( *m_threads.back ()
          , (boost::format ("%1%-%2%") % m_pool_name % i).str ()
          );
      }
    }

    pool_t::~pool_t()
    {
      m_stop = true;

      for (std::size_t i = 0 ; i != m_threads.size () ; ++i)
      {
        m_threads [i]->interrupt ();
        m_threads [i]->join ();
        delete m_threads [i];
      }

      m_threads.clear ();
    }

    void pool_t::worker (size_t rank)
    {
      while (!m_stop || !m_workload.empty ())
      {
        boost::function<void()> w;
        try
        {
          w = m_workload.get ();
        }
        catch (boost::thread_interrupted const &)
        {
          break;
        }

        {
          boost::this_thread::disable_interruption di;

          w();
        }
      }
    }

    void pool_t::execute (boost::function<void()> w)
    {
      m_workload.put (w);
    }

    namespace detail
    {
      struct init_global_pool_t
      {
        init_global_pool_t ()
          : pool (0)
        {
          int ncpu = fhg_get_cpucount ();
          if (ncpu < 0) ncpu = 1;

          pool = new pool_t (ncpu, "global-pool");
        };

        ~init_global_pool_t ()
        {
          delete pool;
          pool = NULL;
        }

        pool_t *pool;
      };
    }

    pool_t & global_pool ()
    {
      static detail::init_global_pool_t gpool;
      return *gpool.pool;
    }
  }
}
