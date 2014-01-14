#ifndef FHG_UTIL_THREAD_POOL_HPP
#define FHG_UTIL_THREAD_POOL_HPP

#include <fhg/util/thread/queue.hpp>

#include <boost/function.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>

namespace fhg
{
  namespace thread
  {
    class pool_t : boost::noncopyable
    {
    public:
      explicit
      pool_t (std::size_t nthread, std::string const &name="threadpool");
      ~pool_t();

      void execute (boost::function<void()> f);
    private:
      void worker();

      boost::ptr_vector<boost::thread> m_threads;
      fhg::thread::queue<boost::function<void()> > m_workload;
    };

    pool_t & global_pool ();
  }
}

#endif
