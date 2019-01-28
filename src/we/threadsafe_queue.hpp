#pragma once

#include <condition_variable>
#include <list>
#include <mutex>

namespace we
{
  template<typename Container>
    class interruptible_threadsafe_queue
  {
    using T = typename std::remove_reference< decltype( std::declval<Container>().front() ) >::type ;

  public:
    interruptible_threadsafe_queue() = default;
    interruptible_threadsafe_queue
      (interruptible_threadsafe_queue<Container> const&) = delete;
    interruptible_threadsafe_queue
      (interruptible_threadsafe_queue<Container>&&) = delete;
    interruptible_threadsafe_queue& operator=
      (interruptible_threadsafe_queue<Container> const&) = delete;
    interruptible_threadsafe_queue& operator=
      (interruptible_threadsafe_queue<Container>&&) = delete;
    ~interruptible_threadsafe_queue() = default;

    T get()
    {
      std::unique_lock<std::mutex> lock (_guard);
      _elements_added_or_interrupted.wait
        (lock, [this] { return !_container.empty() || _interrupted; });

      if (_container.empty()) // _interrupted == true
      {
        std::rethrow_exception( _interrupted_reason.value_or
            ( std::make_exception_ptr (std::logic_error("interrupted not set"))) );
      }

      T element (std::move (_container.front()));
      _container.pop_front();
      return element;
    }

    template<typename... Args> void put ( Args&&... args)
    {
      std::lock_guard<std::mutex> const _ (_guard);

      if (_interrupted)
      {
        std::rethrow_exception( _interrupted_reason.value_or
            ( std::make_exception_ptr (std::logic_error("interrupted not set"))));
      }
      _container.emplace_back (std::forward<Args> (args)...);
      _elements_added_or_interrupted.notify_one();

    }

    template<typename Exception> void interrupt(Exception const& error)
    {
      std::lock_guard<std::mutex> const _ (_guard);
      _interrupted = true;
      _interrupted_reason = std::make_exception_ptr(error);
      _elements_added_or_interrupted.notify_all();
    }


  private:
    mutable std::mutex _guard;
    bool _interrupted = false;
    std::condition_variable _elements_added_or_interrupted;

    boost::optional<std::exception_ptr> _interrupted_reason;
    Container _container;
  };

}
