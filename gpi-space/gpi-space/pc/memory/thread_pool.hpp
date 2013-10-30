#ifndef GPI_SPACE_PC_MEMORY_THREAD_POOL_HPP
#define GPI_SPACE_PC_MEMORY_THREAD_POOL_HPP 1

#include <set>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace gpi
{
  namespace pc
  {
    class thread_pool_t
    {
    public:
      ~thread_pool_t();

      template <typename Fun>
      void add(Fun fun)
      {
        add_thread (new boost::thread(fun));
      }

      template <typename Fun>
      void add(Fun fun, std::size_t num)
      {
        for (std::size_t i (0); i < num; ++i)
        {
          add(fun);
        }
      }

      void interrupt();
      void join();
      void clear();
    private:
      typedef boost::mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      void add_thread(boost::thread* thrd);

      mutable mutex_type m_mutex;
      std::set<boost::thread*> m_pool;
    };
  }
}

#endif
