#include <boost/test/unit_test.hpp>

#include <gspc/Cache.hpp>

#include <util-generic/latch.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/timer/application.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <thread>
#include <unordered_set>
#include <vector>

namespace gspc
{
  using fhg::util::testing::random;
  using fhg::util::testing::randoms;
  using fhg::util::testing::unique_random;

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

  BOOST_AUTO_TEST_CASE (all_slots_can_be_allocated_once)
  {
    auto const N {some_slots()};
    TestCache cache {N};
    std::unordered_set<Slot> slots;

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

    for (auto const data : datas)
    {
      auto const allocation {cache.alloc (data)};

      BOOST_TEST (allocation.state == TestCache::Allocation::Empty);

      BOOST_REQUIRE (slots.emplace (allocation.id).second);
    }

    BOOST_REQUIRE_EQUAL (slots.size(), N);

    while (!slots.empty())
    {
      BOOST_REQUIRE_LT (*slots.begin(), N);
      slots.erase (slots.begin());
    }
  }

  BOOST_AUTO_TEST_CASE (all_slots_can_be_allocated_twice)
  {
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

    for (auto const data : datas)
    {
      cache.alloc (data);
    }

    std::unordered_set<Slot> slots;

    for (auto const data : datas)
    {
      auto const allocation {cache.alloc (data)};

      BOOST_TEST (allocation.state == TestCache::Allocation::Assigned);

      BOOST_REQUIRE (slots.emplace (allocation.id).second);
    }

    BOOST_REQUIRE_EQUAL (slots.size(), N);

    while (!slots.empty())
    {
      BOOST_REQUIRE_LT (*slots.begin(), N);
      slots.erase (slots.begin());
    }
  }

  BOOST_AUTO_TEST_CASE (assigned_is_sticky)
  {
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

    for (auto const data : datas)
    {
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Empty);
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Assigned);
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Assigned);
    }
  }

  BOOST_AUTO_TEST_CASE (remember_can_be_stated_after_first_alloc)
  {
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

    for (auto const data : datas)
    {
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Empty);
      cache.remember (data);
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Remembered);
    }
  }

  BOOST_AUTO_TEST_CASE (remember_can_be_stated_after_second_alloc)
  {
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

    for (auto const data : datas)
    {
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Empty);
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Assigned);
      cache.remember (data);
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Remembered);
    }
  }

  BOOST_AUTO_TEST_CASE (free_before_remember_empties)
  {
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

    for (auto const data : datas)
    {
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Empty);
      cache.free (data);
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Empty);
    }
  }

  BOOST_AUTO_TEST_CASE (remember_is_sticky)
  {
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

    for (auto const data : datas)
    {
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Empty);
      cache.remember (data);
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Remembered);
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Remembered);
    }
  }

  BOOST_AUTO_TEST_CASE (remember_is_sticky_over_free)
  {
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

    for (auto const data : datas)
    {
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Empty);
      cache.remember (data);
      cache.free (data);
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Remembered);
    }
  }

  BOOST_AUTO_TEST_CASE (remember_is_sticky_over_late_free)
  {
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

    for (auto const data : datas)
    {
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Empty);
      cache.remember (data);
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Remembered);
      cache.free (data);
      cache.free (data);
      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Remembered);
    }
  }

  BOOST_AUTO_TEST_CASE (free_unknown_throws)
  {
    TestCache cache {some_slots()};
    auto const data {random<Data>{}()};

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
    TestCache cache {some_slots()};
    auto const data {random<Data>{}()};

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
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

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
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

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
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

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
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

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
    auto const N {some_slots()};
    TestCache cache {N + 1};

    auto const datas {randoms<std::vector<Data>, unique_random> (N + 2)};

    std::unordered_map<Data, Slot> slot;

    for (std::size_t i {0}; i < N + 1; ++i)
    {
      slot.emplace (datas.at (i), cache.alloc (datas.at (i)).id);
      cache.remember (datas.at (i));
    }

    auto const index {random<std::size_t>{}() % (N + 1)};

    cache.free (datas.at (index));

    {
      auto const allocation {cache.alloc (datas.back())};

      BOOST_TEST (allocation.state == TestCache::Allocation::Empty);
      BOOST_REQUIRE_EQUAL (allocation.id, slot.at (datas.at (index)));
    }

    cache.free (datas.back());

    {
      auto const allocation {cache.alloc (datas.at (index))};

      BOOST_TEST (allocation.state == TestCache::Allocation::Empty);
      BOOST_REQUIRE_EQUAL (allocation.id, slot.at (datas.at (index)));
    }

    cache.free (datas.at (index));

    {
      auto const allocation {cache.alloc (datas.back())};

      BOOST_TEST (allocation.state == TestCache::Allocation::Empty);
      BOOST_REQUIRE_EQUAL (allocation.id, slot.at (datas.at (index)));
    }

    cache.remember (datas.back());
    cache.free (datas.back());

    {
      auto const allocation {cache.alloc (datas.at (index))};

      BOOST_TEST (allocation.state == TestCache::Allocation::Empty);
      BOOST_REQUIRE_EQUAL (allocation.id, slot.at (datas.at (index)));
    }
  }

  BOOST_AUTO_TEST_CASE (forget_unknown_is_possible_and_returns_false)
  {
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N + 1)};

    for (std::size_t i {0}; i < N; ++i)
    {
      cache.alloc (datas.at (i));
    }

    BOOST_REQUIRE (!cache.forget (datas.back()));
  }

  BOOST_AUTO_TEST_CASE (forget_when_not_remembered_throws)
  {
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

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
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

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
    auto const N {some_slots()};
    TestCache cache {N};

    auto const datas {randoms<std::vector<Data>, unique_random> (N)};

    for (auto const data : datas)
    {
      cache.alloc (data);
      cache.remember (data);
      cache.free (data);

      BOOST_REQUIRE (cache.forget (data));

      BOOST_TEST (cache.alloc (data).state == TestCache::Allocation::Empty);
    }
  }

  BOOST_AUTO_TEST_CASE (forget_evicted_entry_returns_false)
  {
    auto const N {some_slots()};
    TestCache cache {N + 1};

    auto const datas {randoms<std::vector<Data>, unique_random> (N + 2)};

    for (std::size_t i {0}; i < N + 1; ++i)
    {
      cache.alloc (datas.at (i));
      cache.remember (datas.at (i));
    }

    auto const index {random<std::size_t>{}() % (N + 1)};

    cache.free (datas.at (index));

    cache.alloc (datas.back());

    BOOST_REQUIRE (!cache.forget (datas.at (index)));
  }

  BOOST_AUTO_TEST_CASE (alloc_blocks_until_something_has_been_freed)
  {
    auto const N {some_slots()};
    TestCache cache {N + 1};

    auto const datas {randoms<std::vector<Data>, unique_random> (N + 2)};

    std::unordered_map<Data, Slot> slot;

    for (std::size_t i {0}; i < N + 1; ++i)
    {
      slot.emplace (datas.at (i), cache.alloc (datas.at (i)).id);
    }

    auto const index {random<std::size_t>{}() % (N + 1)};

    fhg::util::latch running (1);

    std::thread thread
      { [&]
        {
          running.count_down();

          auto allocation {cache.alloc (datas.back())};

          BOOST_TEST (allocation.state == TestCache::Allocation::Empty);
          BOOST_REQUIRE_EQUAL (allocation.id, slot.at (datas.at (index)));
        }
      };

    running.wait();

    std::this_thread::sleep_for (std::chrono::milliseconds (300));

    cache.free (datas.at (index));

    thread.join();
  }

  BOOST_AUTO_TEST_CASE (alloc_can_be_interrupted)
  {
    auto const N {some_slots()};
    TestCache cache {N + 1};

    auto const datas {randoms<std::vector<Data>, unique_random> (N + 2)};

    for (std::size_t i {0}; i < N + 1; ++i)
    {
      cache.alloc (datas.at (i));
    }

    fhg::util::latch running (1);

    std::thread thread
      { [&]
        {
          running.count_down();

          BOOST_CHECK_THROW
            ( cache.alloc (datas.back())
            , TestCache::interrupted
            );
        }
      };

    running.wait();

    std::this_thread::sleep_for (std::chrono::milliseconds (300));

    cache.interrupt();

    thread.join();
  }

  BOOST_AUTO_TEST_CASE (interrupt_is_sticky_and_interrupts_all_accesses)
  {
    auto const N {some_slots()};
    TestCache cache {N};

    auto const data {random<Data>{}()};

    cache.interrupt();

    BOOST_CHECK_THROW (cache.alloc (data), TestCache::interrupted);
    BOOST_CHECK_THROW (cache.free (data), TestCache::interrupted);
    BOOST_CHECK_THROW (cache.remember (data), TestCache::interrupted);
    BOOST_CHECK_THROW (cache.forget (data), TestCache::interrupted);
  }

  BOOST_AUTO_TEST_CASE (cache_bigger_example)
  {
    fhg::util::default_application_timer out ("cache_bigger_example");

    auto const N {1 << 20};

    out.section ("Create cache of size " + std::to_string (N));

    TestCache cache {N};

    auto allocate
      { [&] (typename TestCache::Allocation::State expected)
        {
          out.section ("Allocate " + std::to_string (N) + " slots");

          for (Data i {0}; i < N; ++i)
          {
            BOOST_TEST (cache.alloc (i).state == expected);
          }
        }
      };
    auto remember
      { [&]
        {
          out.section ("Remember " + std::to_string (N) + " slots");

          for (Data i {0}; i < N; ++i)
          {
            cache.remember (i);
          }
        }
      };
    auto free
      { [&]
        {
          out.section ("Free " + std::to_string (N) + " slots");

          for (Data i {0}; i < N; ++i)
          {
            cache.free (i);
          }
        }
      };
    auto forget
      { [&]
        {
          out.section ("Forget " + std::to_string (N) + " slots");

          for (Data i {0}; i < N; ++i)
          {
            BOOST_REQUIRE (cache.forget (i));
          }
        }
      };

    allocate (TestCache::Allocation::Empty);
      allocate (TestCache::Allocation::Assigned);
        allocate (TestCache::Allocation::Assigned);
        free();
      free();
    free();

    allocate (TestCache::Allocation::Empty);
      allocate (TestCache::Allocation::Assigned);
        remember();
      free();
      allocate (TestCache::Allocation::Remembered);
      allocate (TestCache::Allocation::Remembered);
        free();
      free();
    free();
    allocate (TestCache::Allocation::Remembered);
    free();

    forget();

    allocate (TestCache::Allocation::Empty);
      remember();
      allocate (TestCache::Allocation::Remembered);
      free();
    free();

    forget();

    allocate (TestCache::Allocation::Empty);
    free();
  }
}
