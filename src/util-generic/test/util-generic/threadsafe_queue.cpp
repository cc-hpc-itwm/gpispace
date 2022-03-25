// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <iostream>
#include <boost/test/unit_test.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/future.hpp>
#include <util-generic/testing/printer/pair.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/threadsafe_queue.hpp>

#include <future>
#include <numeric>

BOOST_AUTO_TEST_CASE (queue_is_fifo)
{
  fhg::util::threadsafe_queue<int> queue;

  auto const first (fhg::util::testing::random<int>{}());
  auto const second (fhg::util::testing::random<int>{}());

  queue.put (first);
  queue.put (second);

  BOOST_REQUIRE_EQUAL (queue.get(), first);
  BOOST_REQUIRE_EQUAL (queue.get(), second);
}

BOOST_AUTO_TEST_CASE (put_many_keeps_order)
{
  fhg::util::threadsafe_queue<int> queue;

  auto const first (fhg::util::testing::random<int>{}());
  auto const second (fhg::util::testing::random<int>{}());
  std::vector<int> data {first, second};

  queue.put_many (data.begin(), data.end());

  BOOST_REQUIRE_EQUAL (queue.get(), first);
  BOOST_REQUIRE_EQUAL (queue.get(), second);
}

BOOST_AUTO_TEST_CASE (get_blocks_if_queue_empty)
{
  fhg::util::threadsafe_queue<int> queue;

  auto got (std::async (std::launch::async, [&] { return queue.get(); }));

  BOOST_REQUIRE_EQUAL ( got.wait_for (std::chrono::milliseconds (100))
                      , std::future_status::timeout
                      );

  auto const value (fhg::util::testing::random<int>{}());
  queue.put (value);

  BOOST_REQUIRE_EQUAL ( got.wait_for (std::chrono::milliseconds (50))
                      , std::future_status::ready
                      );

  BOOST_REQUIRE_EQUAL (got.get(), value);
}

BOOST_AUTO_TEST_CASE (put_many_wakes_many)
{
  fhg::util::threadsafe_queue<int> queue;

  auto got1 (std::async (std::launch::async, [&] { return queue.get(); }));
  auto got2 (std::async (std::launch::async, [&] { return queue.get(); }));

  auto const first (fhg::util::testing::random<int>{}());
  auto const second (fhg::util::testing::random<int>{}());
  std::vector<int> data {first, second};

  queue.put_many (data.begin(), data.end());

  BOOST_REQUIRE_EQUAL (got1.get() + got2.get(), first + second);
}

BOOST_AUTO_TEST_CASE (queue_is_threadsafe)
{
  std::size_t const num_threads (73);

  fhg::util::threadsafe_queue<std::size_t> queue;

  auto const data
    (fhg::util::testing::randoms<std::vector<std::size_t>> (num_threads));

  std::vector<std::future<std::size_t>> getters;
  for (std::size_t id (0); id < num_threads; ++id)
  {
    getters.emplace_back
      (std::async (std::launch::async, [&queue] { return queue.get(); }));
  }

  std::vector<std::future<void>> putters;
  for (std::size_t id (0); id < num_threads; ++id)
  {
    putters.emplace_back
      ( std::async ( std::launch::async
                   , [&queue, &data, id] { return queue.put (data[id]); }
                   )
      );
  }

  for (auto& putter : putters)
  {
    putter.get();
  }

  BOOST_REQUIRE_EQUAL
    ( std::accumulate ( getters.begin()
                      , getters.end()
                      , std::size_t()
                      , [] (std::size_t accum, std::future<std::size_t>& f)
                        {
                          return accum + f.get();
                        }
                      )
    , std::accumulate (data.begin(), data.end(), std::size_t())
    );
}

BOOST_AUTO_TEST_CASE (scoped_backout_takes_first_and_pushes_to_back)
{
  fhg::util::threadsafe_queue<int> queue;

  auto const first (fhg::util::testing::random<int>{}());
  auto const second (fhg::util::testing::random<int>{}());

  queue.put (first);
  queue.put (second);

  {
    fhg::util::threadsafe_queue<int>::scoped_backout const bo (queue);
    BOOST_REQUIRE_EQUAL (first, bo);
  }

  BOOST_REQUIRE_EQUAL (queue.get(), second);
  BOOST_REQUIRE_EQUAL (queue.get(), first);
}

BOOST_AUTO_TEST_CASE (scoped_backout_is_robust_against_modifications_meanwhile)
{
  fhg::util::threadsafe_queue<int> queue;

  auto const first (fhg::util::testing::random<int>{}());
  auto const second (fhg::util::testing::random<int>{}());
  auto const third (fhg::util::testing::random<int>{}());

  queue.put (first);
  queue.put (second);

  {
    fhg::util::threadsafe_queue<int>::scoped_backout const bo (queue);
    BOOST_REQUIRE_EQUAL (first, bo);

    BOOST_REQUIRE_EQUAL (queue.get(), second);
    queue.put (second);
    BOOST_REQUIRE_EQUAL (queue.get(), second);
    queue.put (third);

    {
      fhg::util::threadsafe_queue<int>::scoped_backout const bo2 (queue);
      BOOST_REQUIRE_EQUAL (third, bo2);
    }
  }

  BOOST_REQUIRE_EQUAL (queue.get(), third);
  BOOST_REQUIRE_EQUAL (queue.get(), first);
}

BOOST_AUTO_TEST_CASE (interruption_before_get_is_not_lost)
{
  fhg::util::interruptible_threadsafe_queue<int> queue;

  queue.interrupt();

  BOOST_REQUIRE_THROW (queue.get(), fhg::util::interruptible_threadsafe_queue<int>::interrupted);
}

BOOST_AUTO_TEST_CASE (interruption_during_get_is_delivered)
{
  fhg::util::interruptible_threadsafe_queue<int> queue;

  auto got (std::async (std::launch::async, [&] { return queue.get(); }));

  BOOST_REQUIRE_EQUAL ( got.wait_for (std::chrono::milliseconds (100))
                      , std::future_status::timeout
                      );

  queue.interrupt();

  BOOST_REQUIRE_EQUAL ( got.wait_for (std::chrono::milliseconds (50))
                      , std::future_status::ready
                      );

  BOOST_REQUIRE_THROW (got.get(), fhg::util::interruptible_threadsafe_queue<int>::interrupted);
}

BOOST_AUTO_TEST_CASE (all_getters_are_interrupted)
{
  fhg::util::interruptible_threadsafe_queue<int> queue;

  auto got0 (std::async (std::launch::async, [&] { return queue.get(); }));
  auto got1 (std::async (std::launch::async, [&] { return queue.get(); }));
  auto got2 (std::async (std::launch::async, [&] { return queue.get(); }));

  BOOST_REQUIRE_EQUAL ( got0.wait_for (std::chrono::milliseconds (100))
                      , std::future_status::timeout
                      );
  BOOST_REQUIRE_EQUAL ( got1.wait_for (std::chrono::milliseconds (100))
                      , std::future_status::timeout
                      );
  BOOST_REQUIRE_EQUAL ( got2.wait_for (std::chrono::milliseconds (100))
                      , std::future_status::timeout
                      );

  queue.interrupt();

  BOOST_REQUIRE_THROW (got0.get(), fhg::util::interruptible_threadsafe_queue<int>::interrupted);
  BOOST_REQUIRE_THROW (got1.get(), fhg::util::interruptible_threadsafe_queue<int>::interrupted);
  BOOST_REQUIRE_THROW (got2.get(), fhg::util::interruptible_threadsafe_queue<int>::interrupted);
}

BOOST_AUTO_TEST_CASE (order_and_became_not_full_cond)
{
  std::size_t const num_items (1000);
  std::size_t const capacity (100);

  fhg::util::interruptible_bounded_threadsafe_queue<std::size_t> queue (capacity);

  for (std::size_t k (0); k < num_items; ++k)
  {
    BOOST_REQUIRE ( (queue.try_put (k) && k < capacity)
                  || (!queue.try_put (k) && k >= capacity)
                  );
  }

  BOOST_REQUIRE_EQUAL (queue.get(), std::make_pair (std::size_t (0), true));

  for (std::size_t k (1); k < capacity; ++k)
  {
    BOOST_REQUIRE_EQUAL (queue.get(), std::make_pair (k, false));
  }
}

BOOST_AUTO_TEST_CASE (try_put_by_multiple_threads)
{
  std::size_t const capacity (10);
  std::size_t const num_producers (capacity + 5);

  fhg::util::interruptible_bounded_threadsafe_queue<std::size_t> queue (capacity);

  std::list<std::future<bool>> able_to_puts;

  for (std::size_t k (0); k < num_producers; k++)
  {
    able_to_puts.emplace_back
      ( std::async ( std::launch::async
                   , [&queue, k] { return queue.try_put (k); }
                   )
      );
  }

  std::size_t were_able_to_put (0);
  for (auto& able_to_put : able_to_puts)
  {
    were_able_to_put += (able_to_put.get() ? 1ul : 0ul);
  }

  BOOST_REQUIRE_EQUAL (capacity, were_able_to_put);
}
