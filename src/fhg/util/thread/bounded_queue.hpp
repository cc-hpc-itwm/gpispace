#pragma once

#include <boost/thread/condition_variable.hpp>
#include <list>

namespace fhg
{
  namespace thread
  {
    template <typename T>
      class bounded_queue : public boost::noncopyable
    {
    public:
      bounded_queue (std::size_t capacity)
        :_capacity(capacity)
      {}

      std::pair<T, bool> get()
      {
        boost::unique_lock<boost::mutex> lock (_mtx);
        _cond_non_empty.wait (lock, [this] { return !_container.empty(); });

        T t (_container.front());
        _container.pop_front();

        return std::make_pair (t, _container.size() == _capacity-1);
      }

      template<class... Args>
      bool try_put (Args&&... args)
      {
        boost::unique_lock<boost::mutex> const _ (_mtx);

        if (_container.size() < _capacity)
        {
          _container.emplace_back (std::forward<Args> (args)...);
          _cond_non_empty.notify_one();
          return true;
        }

        return false;
      }

      std::size_t capacity() {return _capacity;}

    private:
      boost::mutex _mtx;
      boost::condition_variable_any _cond_non_empty;

      std::list<T> _container;
      std::size_t _capacity;
    };
  }
}
