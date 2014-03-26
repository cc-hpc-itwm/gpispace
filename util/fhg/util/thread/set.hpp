// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_THREAD_SET_HPP
#define FHG_UTIL_THREAD_SET_HPP

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread.hpp>

#include <algorithm>
#include <functional>

namespace fhg
{
  namespace thread
  {
    //! \note A set of threads which are cleaned up in dtor /  stop_all().
    class set : boost::noncopyable
    {
    public:
      void start (const std::function<void()> fun)
      {
        const boost::mutex::scoped_lock _ (_threads_mutex);
        _threads.push_back (new boost::thread (&set::thread_function, this, fun));
      }

      ~set()
      {
        stop_all();
      }

      //! \todo Should be removed. Exists because of bad dtor in use-case.
      void stop_all()
      {
        const boost::mutex::scoped_lock _ (_threads_mutex);
        for (boost::thread& thread : _threads)
        {
          thread.interrupt();
          if (thread.joinable())
          {
            thread.join();
          }
        }
        _threads.clear();
      }

    private:
      void thread_function (const std::function<void()> fun)
      {
        fun();

        //! \note Remove from set to avoid leaking.
        {
          const boost::mutex::scoped_lock _ (_threads_mutex);
          const boost::ptr_vector<boost::thread>::iterator it
            ( std::find_if ( _threads.begin(), _threads.end()
                           , [] (boost::thread const& t)
                           {
                             return t.get_id() == boost::this_thread::get_id();
                           }
                           )
            );
          if (it != _threads.end())
          {
            _threads.erase (it);
          }
        }
      }

      mutable boost::mutex _threads_mutex;
      boost::ptr_vector<boost::thread> _threads;
    };
  }
}

#endif
