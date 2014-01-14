#include "pool.hpp"

#include <stdexcept>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include <fhg/util/threadname.hpp>
#include <fhg/util/get_cpucount.h>

namespace fhg
{
  namespace thread
  {
    pool_t::pool_t (std::size_t nthread, std::string const &name)
    {
      if (nthread == 0)
      {
        throw std::invalid_argument
          ("fhg::thread::pool_t: nthreads needs to be > 0");
      }

      for (std::size_t i = 0 ; i != nthread ; ++i)
      {
        m_threads.push_back
          (new boost::thread (&pool_t::worker, this, i));
        fhg::util::set_threadname
          ( m_threads.back ()
          , (boost::format ("%1%-%2%") % name % i).str ()
          );
      }
    }

    pool_t::~pool_t()
    {
      BOOST_FOREACH (boost::thread& thread, m_threads)
      {
        thread.interrupt();
        thread.join();
      }
    }

    void pool_t::worker (size_t rank)
    {
      while (true)
      {
        const boost::function<void()> w (m_workload.get());

        const boost::this_thread::disable_interruption _;

        w();
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
