#include <boost/test/unit_test.hpp>

#include <gspc/Cache.hpp>

#include <util-generic/latch.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/timer/application.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <future>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gspc
{
  using fhg::util::testing::random;
  using fhg::util::testing::unique_randoms;

  using Slot = std::uint32_t;
  using Data = std::int32_t;
  using Counter = std::uint16_t;

  using TestCache = Cache<Slot, Data, Counter>;

  namespace
  {
    Slot some_slots()
    {
      return random<Slot>{}() % 1000;
    }
  }

#define REQUIRE_ALLOCATION(what_, refcount_, remembered_)      \
  do                                                           \
  {                                                            \
    auto const to_check (what_);                               \
    BOOST_TEST (to_check.reference_count == refcount_);        \
    BOOST_TEST (to_check.was_remembered == remembered_);       \
  }                                                            \
  while (false)

  BOOST_AUTO_TEST_CASE (all_slots_can_be_allocated_once)
  {
    auto const N (some_slots());
    TestCache cache (N);
    std::unordered_set<Slot> slots;

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      auto const allocation (cache.alloc (data));

      REQUIRE_ALLOCATION (allocation, 1, false);

      BOOST_REQUIRE (slots.emplace (allocation.id).second);
      BOOST_REQUIRE_LT (allocation.id, N);
    }
  }

  BOOST_AUTO_TEST_CASE (all_slots_can_be_allocated_twice)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      cache.alloc (data);
    }

    std::unordered_set<Slot> slots;

    for (auto const data : datas)
    {
      auto const allocation (cache.alloc (data));

      REQUIRE_ALLOCATION (allocation, 2, false);

      BOOST_REQUIRE (slots.emplace (allocation.id).second);
      BOOST_REQUIRE_LT (allocation.id, N);
    }
  }

  BOOST_AUTO_TEST_CASE (assigned_is_sticky)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      REQUIRE_ALLOCATION (cache.alloc (data), 1, false);
      REQUIRE_ALLOCATION (cache.alloc (data), 2, false);
      REQUIRE_ALLOCATION (cache.alloc (data), 3, false);
    }
  }

  BOOST_AUTO_TEST_CASE (remember_can_be_stated_after_first_alloc)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      REQUIRE_ALLOCATION (cache.alloc (data), 1, false);
      cache.remember (data);
      REQUIRE_ALLOCATION (cache.alloc (data), 2, true);
    }
  }

  BOOST_AUTO_TEST_CASE (remember_can_be_stated_after_second_alloc)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      REQUIRE_ALLOCATION (cache.alloc (data), 1, false);
      REQUIRE_ALLOCATION (cache.alloc (data), 2, false);
      cache.remember (data);
      REQUIRE_ALLOCATION (cache.alloc (data), 3, true);
    }
  }

  BOOST_AUTO_TEST_CASE (free_before_remember_empties)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      REQUIRE_ALLOCATION (cache.alloc (data), 1, false);
      cache.free (data);
      REQUIRE_ALLOCATION (cache.alloc (data), 1, false);
    }
  }

  BOOST_AUTO_TEST_CASE (remember_is_sticky)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      REQUIRE_ALLOCATION (cache.alloc (data), 1, false);
      cache.remember (data);
      REQUIRE_ALLOCATION (cache.alloc (data), 2, true);
      REQUIRE_ALLOCATION (cache.alloc (data), 3, true);
    }
  }

  BOOST_AUTO_TEST_CASE (remember_is_sticky_over_free)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      REQUIRE_ALLOCATION (cache.alloc (data), 1, false);
      cache.remember (data);
      cache.free (data);
      REQUIRE_ALLOCATION (cache.alloc (data), 1, true);
    }
  }

  BOOST_AUTO_TEST_CASE (remember_is_sticky_over_late_free)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      REQUIRE_ALLOCATION (cache.alloc (data), 1, false);
      cache.remember (data);
      REQUIRE_ALLOCATION (cache.alloc (data), 2, true);
      cache.free (data);
      cache.free (data);
      REQUIRE_ALLOCATION (cache.alloc (data), 1, true);
    }
  }

  BOOST_AUTO_TEST_CASE (free_unknown_throws)
  {
    TestCache cache (some_slots());
    auto const data (random<Data>{}());

    fhg::util::testing::require_exception
      ( [&]
        {
          cache.free (data);
        }
      , fhg::util::testing::make_nested
        ( std::runtime_error
          ( ( boost::format ("Cache::free (%1%)")
            % data
            ).str()
          )
        , std::invalid_argument ("Unknown")
        )
      );
  }

  BOOST_AUTO_TEST_CASE (remember_unknown_throws)
  {
    TestCache cache (some_slots());
    auto const data (random<Data>{}());

    fhg::util::testing::require_exception
      ( [&]
        {
          cache.remember (data);
        }
      , fhg::util::testing::make_nested
        ( std::runtime_error
          ( ( boost::format ("Cache::remember (%1%)")
            % data
            ).str()
          )
        , std::invalid_argument ("Unknown")
        )
      );
  }

  BOOST_AUTO_TEST_CASE (second_remember_throws)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      cache.alloc (data);
      cache.remember (data);

      fhg::util::testing::require_exception
        ( [&]
          {
            cache.remember (data);
          }
        , fhg::util::testing::make_nested
          ( std::runtime_error
            ( ( boost::format ("Cache::remember (%1%)")
              % data
              ).str()
            )
          , std::logic_error ("Entry::remember: Already remembered")
          )
        );
    }
  }

  BOOST_AUTO_TEST_CASE (second_free_before_remember_throws_unknown)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      cache.alloc (data);
      cache.free (data);

      fhg::util::testing::require_exception
        ( [&]
          {
            cache.free (data);
          }
        , fhg::util::testing::make_nested
          ( std::runtime_error
            ( ( boost::format ("Cache::free (%1%)")
              % data
              ).str()
            )
          , std::invalid_argument ("Unknown")
          )
        );
    }
  }

  BOOST_AUTO_TEST_CASE (second_free_after_remember_throws)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      cache.alloc (data);
      cache.remember (data);
      cache.free (data);

      fhg::util::testing::require_exception
        ( [&]
          {
            cache.free (data);
          }
        , fhg::util::testing::make_nested
          ( std::runtime_error
            ( ( boost::format ("Cache::free (%1%)")
              % data
              ).str()
            )
          , std::logic_error ("References::decrement: Not in use")
          )
        );
    }
  }

  BOOST_AUTO_TEST_CASE (counter_overflow_is_signalled)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      for (Counter c {0}; c < std::numeric_limits<Counter>::max(); ++c)
      {
        cache.alloc (data);
      }

      fhg::util::testing::require_exception
        ( [&]
          {
            cache.alloc (data);
          }
        , fhg::util::testing::make_nested
          ( std::runtime_error
            ( ( boost::format ("Cache::alloc (%1%)")
              % data
              ).str()
            )
          , std::logic_error ("References::increment: Overflow")
          )
        );
    }
  }

  BOOST_AUTO_TEST_CASE (unused_data_is_evicted)
  {
    auto const N (some_slots());
    TestCache cache (N + 1);

    auto const datas (unique_randoms<std::vector<Data>> (N + 2));

    std::unordered_map<Data, Slot> slot;

    for (std::size_t i {0}; i < N + 1; ++i)
    {
      slot.emplace (datas.at (i), cache.alloc (datas.at (i)).id);
      cache.remember (datas.at (i));
    }

    auto const index (random<std::size_t>{}() % (N + 1));

    cache.free (datas.at (index));

    {
      auto const allocation (cache.alloc (datas.back()));

      REQUIRE_ALLOCATION (allocation, 1, false);
      BOOST_REQUIRE_EQUAL (allocation.id, slot.at (datas.at (index)));
    }

    cache.free (datas.back());

    {
      auto const allocation (cache.alloc (datas.at (index)));

      REQUIRE_ALLOCATION (allocation, 1, false);
      BOOST_REQUIRE_EQUAL (allocation.id, slot.at (datas.at (index)));
    }

    cache.free (datas.at (index));

    {
      auto const allocation (cache.alloc (datas.back()));

      REQUIRE_ALLOCATION (allocation, 1, false);
      BOOST_REQUIRE_EQUAL (allocation.id, slot.at (datas.at (index)));
    }

    cache.remember (datas.back());
    cache.free (datas.back());

    {
      auto const allocation (cache.alloc (datas.at (index)));

      REQUIRE_ALLOCATION (allocation, 1, false);
      BOOST_REQUIRE_EQUAL (allocation.id, slot.at (datas.at (index)));
    }
  }

  BOOST_AUTO_TEST_CASE (forget_unknown_is_possible_and_returns_false)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N + 1));

    for (std::size_t i {0}; i < N; ++i)
    {
      cache.alloc (datas.at (i));
    }

    BOOST_REQUIRE (!cache.forget (datas.back()));
  }

  BOOST_AUTO_TEST_CASE (forget_when_not_remembered_throws)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      cache.alloc (data);

      fhg::util::testing::require_exception
        ( [&]
          {
            cache.forget (data);
          }
        , fhg::util::testing::make_nested
          ( std::runtime_error
            ( ( boost::format ("Cache::forget (%1%)")
              % data
              ).str()
            )
          , std::invalid_argument ("Not remembered")
          )
        );
    }
  }

  BOOST_AUTO_TEST_CASE (forget_when_in_use_throws)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      cache.alloc (data);
      cache.remember (data);

      fhg::util::testing::require_exception
        ( [&]
          {
            cache.forget (data);
          }
        , fhg::util::testing::make_nested
          ( std::runtime_error
            ( ( boost::format ("Cache::forget (%1%)")
              % data
              ).str()
            )
          , std::invalid_argument ("In use")
          )
        );
    }
  }

  BOOST_AUTO_TEST_CASE (forget_known_entry_forgets)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const datas (unique_randoms<std::vector<Data>> (N));

    for (auto const data : datas)
    {
      cache.alloc (data);
      cache.remember (data);
      cache.free (data);

      BOOST_REQUIRE (cache.forget (data));

      REQUIRE_ALLOCATION (cache.alloc (data), 1, false);
    }
  }

  BOOST_AUTO_TEST_CASE (forget_evicted_entry_returns_false)
  {
    auto const N (some_slots());
    TestCache cache (N + 1);

    auto const datas (unique_randoms<std::vector<Data>> (N + 2));

    for (std::size_t i {0}; i < N + 1; ++i)
    {
      cache.alloc (datas.at (i));
      cache.remember (datas.at (i));
    }

    auto const index (random<std::size_t>{}() % (N + 1));

    cache.free (datas.at (index));

    cache.alloc (datas.back());

    BOOST_REQUIRE (!cache.forget (datas.at (index)));
  }

  BOOST_AUTO_TEST_CASE (alloc_blocks_until_something_has_been_freed)
  {
    auto const N (some_slots());
    TestCache cache (N + 1);

    auto const datas (unique_randoms<std::vector<Data>> (N + 2));

    std::unordered_map<Data, Slot> slot;

    for (std::size_t i {0}; i < N + 1; ++i)
    {
      slot.emplace (datas.at (i), cache.alloc (datas.at (i)).id);
    }

    auto const index (random<std::size_t>{}() % (N + 1));

    fhg::util::latch running (1);

    auto allocation_future
      { std::async ( std::launch::async
                   , [&]
                     {
                       running.count_down();

                       return cache.alloc (datas.back());
                     }
                   )
      };

    running.wait();

    //! \note Race in the test here: free() may be called before
    //! alloc(). This sleep tries to make the race less likely.
    std::this_thread::sleep_for (std::chrono::milliseconds (300));

    cache.free (datas.at (index));

    auto const allocation (allocation_future.get());
    REQUIRE_ALLOCATION (allocation, 1, false);
    BOOST_REQUIRE_EQUAL (allocation.id, slot.at (datas.at (index)));
  }

  BOOST_AUTO_TEST_CASE (alloc_can_be_interrupted)
  {
    auto const N (some_slots());
    TestCache cache (N + 1);

    auto const datas (unique_randoms<std::vector<Data>> (N + 2));

    for (std::size_t i {0}; i < N + 1; ++i)
    {
      cache.alloc (datas.at (i));
    }

    fhg::util::latch running (1);

    auto allocation_future
      { std::async ( std::launch::async
                   , [&]
                     {
                       running.count_down();

                       cache.alloc (datas.back());
                     }
                   )
      };

    running.wait();

    //! \note Race in the test here: interrupt() may be called before
    //! alloc(). This sleep tries to make the race less likely.
    std::this_thread::sleep_for (std::chrono::milliseconds (300));

    cache.interrupt();

    BOOST_CHECK_THROW (allocation_future.get(), TestCache::interrupted);
  }

  BOOST_AUTO_TEST_CASE (interrupt_is_sticky_and_interrupts_all_accesses)
  {
    auto const N (some_slots());
    TestCache cache (N);

    auto const data (random<Data>{}());

    cache.interrupt();

    BOOST_CHECK_THROW (cache.alloc (data), TestCache::interrupted);
    BOOST_CHECK_THROW (cache.free (data), TestCache::interrupted);
    BOOST_CHECK_THROW (cache.remember (data), TestCache::interrupted);
    BOOST_CHECK_THROW (cache.forget (data), TestCache::interrupted);
  }

  BOOST_AUTO_TEST_CASE (cache_bigger_example)
  {
    fhg::util::default_application_timer out ("cache_bigger_example");

    auto const N (1 << 20);

    out.section ("Create cache of size " + std::to_string (N));

    TestCache cache (N);


#define allocate(expected_refcount_, expected_remembered_)              \
    do                                                                  \
    {                                                                   \
      out.section ("Allocate " + std::to_string (N) + " slots");        \
                                                                        \
      for (Data i {0}; i < N; ++i)                                      \
      {                                                                 \
        REQUIRE_ALLOCATION                                              \
          (cache.alloc (i), expected_refcount_, expected_remembered_);  \
      }                                                                 \
    }                                                                   \
    while (false)
#define remember()                                                      \
    do                                                                  \
    {                                                                   \
      out.section ("Remember " + std::to_string (N) + " slots");        \
                                                                        \
      for (Data i {0}; i < N; ++i)                                      \
      {                                                                 \
        cache.remember (i);                                             \
      }                                                                 \
    }                                                                   \
    while (false)
#define free()                                                          \
    do                                                                  \
    {                                                                   \
      out.section ("Free " + std::to_string (N) + " slots");            \
                                                                        \
      for (Data i {0}; i < N; ++i)                                      \
      {                                                                 \
        cache.free (i);                                                 \
      }                                                                 \
    }                                                                   \
    while (false)
#define forget()                                                        \
    do                                                                  \
    {                                                                   \
      out.section ("Forget " + std::to_string (N) + " slots");          \
                                                                        \
      for (Data i {0}; i < N; ++i)                                      \
      {                                                                 \
        BOOST_REQUIRE (cache.forget (i));                               \
      }                                                                 \
    }                                                                   \
    while (false)

    allocate (1, false);
      allocate (2, false);
        allocate (3, false);
        free();
      free();
    free();

    allocate (1, false);
      allocate (2, false);
        remember();
      free();
      allocate (2, true);
      allocate (3, true);
        free();
      free();
    free();
    allocate (1, true);
    free();

    forget();

    allocate (1, false);
      remember();
      allocate (2, true);
      free();
    free();

    forget();

    allocate (1, false);
    free();
  }
}
