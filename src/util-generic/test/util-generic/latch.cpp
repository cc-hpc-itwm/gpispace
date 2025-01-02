// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <util-generic/latch.hpp>

#include <util-generic/testing/require_exception.hpp>

#include <future>
#include <memory>
#include <thread>

BOOST_AUTO_TEST_CASE (latch_zero_constructible_and_can_be_waited_for)
{
  fhg::util::latch (0).wait();
}

BOOST_AUTO_TEST_CASE (counting_when_reached_zero_is_logic_error)
{
  fhg::util::testing::require_exception
    ( [] { fhg::util::latch (0).count_down(); }
    , fhg::util::error::count_down_for_zero()
    );
}

BOOST_AUTO_TEST_CASE (one_thread_waits_the_other_counts_down)
{
  fhg::util::latch latch (1);

  std::thread waiter (&fhg::util::latch::wait, &latch);
  std::thread counter (&fhg::util::latch::count_down, &latch);

  waiter.join();
  counter.join();
}

BOOST_AUTO_TEST_CASE (one_thread_can_count_down_and_wait)
{
  fhg::util::latch latch (1);

  latch.count_down();
  latch.wait();
}

BOOST_AUTO_TEST_CASE (many_counters)
{
  std::size_t num_threads (793);
  fhg::util::latch latch (num_threads);

  std::vector<std::thread> counters;
  while (num_threads --> 0)
  {
    counters.emplace_back (&fhg::util::latch::count_down, &latch);
  }

  latch.wait();

  for (std::thread& counter : counters)
  {
    counter.join();
  }
}

BOOST_AUTO_TEST_CASE (many_waiters)
{
  std::size_t num_threads (793);
  fhg::util::latch latch (1);

  std::vector<std::thread> waiters;
  while (num_threads --> 0)
  {
    waiters.emplace_back (&fhg::util::latch::wait, &latch);
  }

  latch.count_down();

  for (std::thread& waiter : waiters)
  {
    waiter.join();
  }
}

BOOST_AUTO_TEST_CASE (many_countdowns_from_a_single_thread)
{
  std::size_t const N (111);

  fhg::util::latch latch (N);

  std::thread waiter (&fhg::util::latch::wait, &latch);
  std::thread counter
    ([&latch]
     {
       auto count (N);

       while (count --> 0)
       {
         latch.count_down();
       }
     }
    );

  waiter.join();
  counter.join();
}

BOOST_AUTO_TEST_CASE (multiple_countdown_in_multiple_threads)
{
  std::size_t const num_threads (61);
  std::size_t const N (num_threads * 13);
  fhg::util::latch latch (N);

  std::vector<std::thread> counters;

  for (std::size_t thread (0); thread < num_threads; ++thread)
  {
    counters.emplace_back
      ( [thread, &latch]
        {
          for (std::size_t n (thread); n < N; n += num_threads)
          {
            latch.count_down();
          }
        }
      );
  }

  latch.wait();

  for (std::thread& counter : counters)
  {
    counter.join();
  }
}

namespace
{
  struct barrier
  {
    barrier (std::size_t num_threads)
      : _notify (num_threads)
      , _release (1)
    {}
    void wait()
    {
      _notify.wait();
    }
    void release()
    {
      _release.count_down();
    }
    void operator() ()
    {
      _notify.count_down();
      _release.wait();
    }
  private:
    fhg::util::latch _notify;
    fhg::util::latch _release;
  };
}

BOOST_AUTO_TEST_CASE (two_latches_form_a_startup_barrier)
{
  std::size_t const num_threads (793);
  struct barrier barrier (num_threads);

  std::unique_ptr<fhg::util::latch> counter;

  std::vector<std::thread> threads;

  for (std::size_t thread (0); thread < num_threads; ++thread)
  {
    threads.emplace_back
      ([&barrier, &counter] { barrier(); counter->count_down(); });
  }

  barrier.wait();
  counter = std::make_unique<fhg::util::latch> (num_threads);
  barrier.release();

  counter->wait();

  for (std::thread& thread : threads)
  {
    thread.join();
  }
}

BOOST_AUTO_TEST_CASE (wait_and_reset_allows_for_reuse)
{
  fhg::util::latch latch (0);
  latch.wait_and_reset (0);
  latch.wait();

  latch.wait_and_reset (1);

  {
    std::thread waiter (&fhg::util::latch::wait, &latch);
    std::thread counter (&fhg::util::latch::count_down, &latch);

    waiter.join();
    counter.join();
  }

  latch.wait_and_reset (1);

  {
    std::thread waiter_a ( [&]
                           {
                             latch.wait_and_reset (1);
                             latch.count_down();
                           }
                         );
    std::thread waiter_b ( [&]
                           {
                             latch.wait_and_reset (1);
                             latch.count_down();
                           }
                         );

    std::thread counter (&fhg::util::latch::count_down, &latch);

    waiter_b.join();
    waiter_a.join();
    counter.join();

    latch.wait();
  }
}

namespace fhg
{
  namespace util
  {
    //! \note This is a quite rare race in this context. It was more
    //! pronounced in shared/rpc test case for issue 19. It can be
    //! provoked more often by adding a sleep/yield in
    //! `latch::count_down` before the `notify_all` (without the fix
    //! of keeping the mutex, of course). With the 0xCC pattern it
    //! seems to rather timeout than segfault, but it is undefined
    //! behaviour after all (libpthread does weird magic constant
    //! magic in mutex locking, so there might be "better" patterns
    //! than 0xCC if wanting a segfault). With a sleep in there, it
    //! usually hangs on the first iteration already. Without, it is
    //! quite some luck with the kernel scheduling a yield there.
    BOOST_AUTO_TEST_CASE
      ( shall_not_segfault_or_timeout_if_destructed_after_wait_on_countdown
      , *::boost::unit_test::timeout (10)
      )
    {
      std::aligned_storage<sizeof (latch), alignof (latch)>::type storage;
      latch* latch_ (static_cast<latch*> (static_cast<void*> (&storage)));

      for (std::size_t i (0); i < 1000; ++i)
      {
        new (latch_) latch (1);

        auto cd (std::async (std::launch::async, &latch::count_down, latch_));

        latch_->wait();

        //! \note emulate destruction with memory repurpose
        latch_->~latch();
        memset (&storage, 0xCC, sizeof (storage));

        cd.get();
      }
    }
  }
}
