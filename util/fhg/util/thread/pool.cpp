#include <fhg/util/thread/pool.hpp>

#include <stdexcept>

#include <boost/foreach.hpp>

namespace fhg
{
  namespace thread
  {
    pool_t::pool_t (std::size_t nthread)
    {
      if (nthread == 0)
      {
        throw std::invalid_argument
          ("fhg::thread::pool_t: nthreads needs to be > 0");
      }

      for (std::size_t i = 0 ; i != nthread ; ++i)
      {
        m_threads.push_back (new boost::thread (&pool_t::worker, this));
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

    void pool_t::worker()
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
  }
}
