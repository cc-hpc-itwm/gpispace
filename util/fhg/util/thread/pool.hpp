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
      typedef boost::function<void (void)> work_t;
      typedef boost::function<void (void)> callback_t;

      explicit
      pool_t (std::size_t nthread, std::string const &name="threadpool");
      ~pool_t();

      void execute (work_t f);
      void execute (work_t f, callback_t cb);
    private:
      void worker (size_t rank);

      typedef std::vector<boost::thread *>    thread_list_t;
      typedef std::pair<work_t, callback_t>   work_item_t;
      typedef fhg::thread::queue<work_item_t> work_queue_t;

      bool          m_stop;
      std::size_t   m_nthread;
      std::string   m_pool_name;
      thread_list_t m_threads;
      work_queue_t  m_workload;
    };

    pool_t & global_pool ();
  }
}

#endif
