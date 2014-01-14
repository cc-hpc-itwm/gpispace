#ifndef FHG_UTIL_THREAD_POOL_HPP
#define FHG_UTIL_THREAD_POOL_HPP

#include <list>
#include <vector>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/utility.hpp>

#include <fhg/util/thread/queue.hpp>

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
      void worker (size_t rank);

      typedef std::vector<boost::thread *>    thread_list_t;

      thread_list_t m_threads;
      fhg::thread::queue<boost::function<void()> > m_workload;
    };

    pool_t & global_pool ();
  }
}

#endif
