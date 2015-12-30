#pragma once

#include <boost/thread.hpp>

namespace fhg
{
  namespace util
  {
    namespace thread
    {
      template <typename T = void> class event;

      template <typename T>
        class event : boost::noncopyable
      {
        T _event;
        bool _signalled {false};
        mutable boost::mutex _mutex;
        mutable boost::condition_variable _condition;

      public:
        T wait()
        {
          boost::mutex::scoped_lock lock (_mutex);

          _condition.wait (lock, [this] { return _signalled; });
          _signalled = false;

          return std::move (_event);
        }

        void notify (T u)
        {
          boost::mutex::scoped_lock const _ (_mutex);

          _event = std::move (u);

          _signalled = true;
          _condition.notify_one();
        }
      };

      template<>
        class event<void> : boost::noncopyable
      {
        bool _signalled {false};
        mutable boost::mutex _mutex;
        mutable boost::condition_variable _condition;

      public:
        void wait()
        {
          boost::mutex::scoped_lock lock (_mutex);

          _condition.wait (lock, [this] { return _signalled; });
          _signalled = false;
        }

        void notify()
        {
          boost::mutex::scoped_lock const _ (_mutex);

          _signalled = true;
          _condition.notify_one();
        }
      };
    }
  }
}
