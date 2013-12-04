// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_THREAD_SET_HPP
#define FHG_UTIL_THREAD_SET_HPP

#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread.hpp>

#include <algorithm>

namespace fhg
{
  namespace thread
  {
    //! \note A set of threads which are cleaned up in dtor /  stop_all().
    class set : boost::noncopyable
    {
    public:
      void start (const boost::function<void()> fun)
      {
        boost::mutex::scoped_lock _ (_threads_mutex);
        _threads.push_back (new boost::thread (&set::thread_function, this, fun));
      }

      ~set()
      {
        stop_all();
      }

      //! \todo Should be removed. Exists because of bad dtor in use-case.
      void stop_all()
      {
        boost::mutex::scoped_lock _ (_threads_mutex);
        BOOST_FOREACH (boost::thread& thread, _threads)
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
      void thread_function (const boost::function<void()> fun)
      {
        fun();

        //! \note Remove from set to avoid leaking.
        {
          boost::mutex::scoped_lock _ (_threads_mutex);
          const boost::ptr_vector<boost::thread>::iterator it
            ( std::find_if ( _threads.begin(), _threads.end()
                           , boost::bind (&boost::thread::get_id, _1)
                           == boost::this_thread::get_id()
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
