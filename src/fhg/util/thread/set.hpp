#pragma once

#include <util-generic/cxx14/make_unique.hpp>

#include <algorithm>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace fhg
{
  namespace thread
  {
    //! \note A set of threads which are cleaned up in dtor.
    class set : boost::noncopyable
    {
    public:
      void start (const std::function<void()> fun)
      {
        if (_stopped)
        {
          return;
        }

        const std::lock_guard<std::mutex> _ (_threads_mutex);
        _threads.emplace_back ( fhg::util::cxx14::make_unique<std::thread>
                                  (&set::thread_function, this, fun)
                              );
      }

      ~set()
      {
        std::vector<std::unique_ptr<std::thread>> threads;
        {
          std::lock_guard<std::mutex> const _ (_threads_mutex);
          _stopped = true;
          std::swap (threads, _threads);
        }

        for (std::unique_ptr<std::thread>& thread : threads)
        {
          if (thread->joinable())
          {
            thread->join();
          }
        }
      }

    private:
      void thread_function (const std::function<void()> fun)
      {
        fun();

        //! \note Remove from set to avoid leaking.
        {
          const std::lock_guard<std::mutex> _ (_threads_mutex);
          const std::vector<std::unique_ptr<std::thread>>::iterator it
            ( std::find_if ( _threads.begin(), _threads.end()
                           , [] (std::unique_ptr<std::thread> const& t)
                           {
                             return t->get_id() == std::this_thread::get_id();
                           }
                           )
            );
          if (it != _threads.end())
          {
            _threads.erase (it);
          }
        }
      }

      mutable std::mutex _threads_mutex;
      std::vector<std::unique_ptr<std::thread>> _threads;
      bool _stopped = false;
    };
  }
}
