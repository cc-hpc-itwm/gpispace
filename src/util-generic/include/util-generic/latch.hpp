// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <condition_variable>
#include <mutex>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace error
    {
      struct count_down_for_zero : std::logic_error
      {
        count_down_for_zero();
      };
    }

    //! A threadsafe countdown which offers to wait for count reaching
    //! zero from multiple threads.
    struct latch
    {
    public:
      //! \note 0 is a valid count.
      latch (std::size_t count);

      //! Blocks until count reaches zero. May be called from multiple
      //! threads.
      void wait();
      //! Like wait(), but will thread-safe reset the counter to the
      //! given value.
      //! \note While it is threadsafe, calling reset() from multiple
      //! threads is highly likely a bug as it is unspecified which
      //! thread would "win" the reset and thus one couldn't
      //! "correctly" count_down() if the two resets had different
      //! values to reset to. If the values are equal though,
      //! everything is well defined and fine (e.g. when wanting to
      //! trigger an event every n ticks, and having multiple threads
      //! that can do the event, all `wait_and_reset (n)`ing all the
      //! time (except that ticks would need to be paused when a
      //! `wait()` was triggered, which is probably hard).
      void wait_and_reset (std::size_t);
      //! \throws error::count_down_for_zero if the counter is at 0 already.
      void count_down();

      latch (latch const&) = delete;
      latch (latch&&) = delete;
      latch& operator= (latch const&) = delete;
      latch& operator= (latch&&) = delete;
      ~latch() = default;

    private:
      std::mutex _guard;
      std::condition_variable _counted_down;
      std::size_t _count;
    };
  }
}

#include <util-generic/latch.ipp>
