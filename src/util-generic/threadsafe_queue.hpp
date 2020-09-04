// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <condition_variable>
#include <list>
#include <mutex>

namespace fhg
{
  namespace util
  {
    template<typename T>
      class threadsafe_queue
    {
    public:
      threadsafe_queue() = default;
      threadsafe_queue (threadsafe_queue<T> const&) = delete;
      threadsafe_queue (threadsafe_queue<T>&&) = delete;
      threadsafe_queue& operator= (threadsafe_queue<T> const&) = delete;
      threadsafe_queue& operator= (threadsafe_queue<T>&&) = delete;
      ~threadsafe_queue() = default;

      T get()
      {
        std::unique_lock<std::mutex> lock (_guard);
        _elements_added.wait (lock, [this] { return !_container.empty(); });

        T element (std::move (_container.front()));
        _container.pop_front();
        return element;
      }

      template<typename... Args> void put (Args&&... args)
      {
        std::lock_guard<std::mutex> const _ (_guard);
        _container.emplace_back (std::forward<Args> (args)...);
        _elements_added.notify_one();
      }

      template<typename FwdIt> void put_many (FwdIt begin, FwdIt end)
      {
        std::lock_guard<std::mutex> const _ (_guard);
        _container.insert (_container.end(), begin, end);
        _elements_added.notify_all();
      }

      struct scoped_backout
      {
        scoped_backout (threadsafe_queue<T>& queue)
          : _queue (queue)
          , _element (std::move (_queue.get()))
        {}
        ~scoped_backout()
        {
          _queue.put (std::move (_element));
        }

        operator T const&() const { return _element; }
        T const* operator->() const { return &_element; }

        scoped_backout (scoped_backout const&) = delete;
        scoped_backout (scoped_backout&&) = delete;
        scoped_backout& operator= (scoped_backout const&) = delete;
        scoped_backout& operator= (scoped_backout&&) = delete;

      public:
        threadsafe_queue<T>& _queue;
        T _element;
      };

    private:
      mutable std::mutex _guard;
      std::condition_variable _elements_added;

      std::list<T> _container;
    };

    template<typename T>
      class interruptible_threadsafe_queue
    {
    public:
      interruptible_threadsafe_queue() = default;
      interruptible_threadsafe_queue
        (interruptible_threadsafe_queue<T> const&) = delete;
      interruptible_threadsafe_queue
        (interruptible_threadsafe_queue<T>&&) = delete;
      interruptible_threadsafe_queue& operator=
        (interruptible_threadsafe_queue<T> const&) = delete;
      interruptible_threadsafe_queue& operator=
        (interruptible_threadsafe_queue<T>&&) = delete;
      ~interruptible_threadsafe_queue() = default;

      T get()
      {
        std::unique_lock<std::mutex> lock (_guard);
        _elements_added_or_interrupted.wait
          (lock, [this] { return !_container.empty() || _interrupted; });

        if (_interrupted)
        {
          throw interrupted();
        }

        T element (std::move (_container.front()));
        _container.pop_front();
        return element;
      }

      template<typename... Args> void put (Args&&... args)
      {
        std::lock_guard<std::mutex> const _ (_guard);
        _container.emplace_back (std::forward<Args> (args)...);
        _elements_added_or_interrupted.notify_one();
      }

      struct interrupted {};
      void interrupt()
      {
        std::lock_guard<std::mutex> const _ (_guard);
        _interrupted = true;
        _elements_added_or_interrupted.notify_all();
      }

      struct interrupt_on_scope_exit
      {
        interrupt_on_scope_exit (interruptible_threadsafe_queue<T>& queue)
          : _queue (queue)
        {}
        ~interrupt_on_scope_exit()
        {
          _queue.interrupt();
        }

      private:
        interruptible_threadsafe_queue<T>& _queue;
      };

    private:
      mutable std::mutex _guard;
      bool _interrupted = false;
      std::condition_variable _elements_added_or_interrupted;

      std::list<T> _container;
    };

    template<typename T>
      class interruptible_bounded_threadsafe_queue
    {
    public:
      interruptible_bounded_threadsafe_queue (std::size_t capacity)
        : _capacity (capacity)
      {}

      interruptible_bounded_threadsafe_queue
        (interruptible_bounded_threadsafe_queue<T> const&) = delete;
      interruptible_bounded_threadsafe_queue
        (interruptible_bounded_threadsafe_queue<T>&&) = delete;
      interruptible_bounded_threadsafe_queue& operator=
        (interruptible_bounded_threadsafe_queue<T> const&) = delete;
      interruptible_bounded_threadsafe_queue& operator=
        (interruptible_bounded_threadsafe_queue<T>&&) = delete;
      ~interruptible_bounded_threadsafe_queue() = default;

      std::pair<T, bool> get()
      {
        std::unique_lock<std::mutex> lock (_guard);
        _elements_added_or_interrupted.wait
          (lock, [this] { return !_container.empty() || _interrupted; });

        if (_interrupted)
        {
          throw interrupted();
        }

        bool const was_full (_container.size() == _capacity);

        T element (std::move (_container.front()));
        _container.pop_front();
        return {std::move (element), was_full};
      }

      template<typename... Args> bool try_put (Args&&... args)
      {
        std::lock_guard<std::mutex> const _ (_guard);

        if (_container.size() >= _capacity)
        {
          return false;
        }

        _container.emplace_back (std::forward<Args> (args)...);
        _elements_added_or_interrupted.notify_one();

        return true;
      }

      struct interrupted {};
      void interrupt()
      {
        std::lock_guard<std::mutex> const _ (_guard);
        _interrupted = true;
        _elements_added_or_interrupted.notify_all();
      }

      struct interrupt_on_scope_exit
      {
        interrupt_on_scope_exit
            (interruptible_bounded_threadsafe_queue<T>& queue)
          : _queue (queue)
        {}
        ~interrupt_on_scope_exit()
        {
          _queue.interrupt();
        }

      private:
        interruptible_bounded_threadsafe_queue<T>& _queue;
      };

    private:
      std::size_t _capacity;

      mutable std::mutex _guard;
      bool _interrupted = false;
      std::condition_variable _elements_added_or_interrupted;

      std::list<T> _container;
    };
  }
}
