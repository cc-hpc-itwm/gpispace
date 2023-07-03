// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/latch.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <chrono>
#include <thread>

BOOST_AUTO_TEST_CASE
  (zero_threads_are_fine_if_just_not_used, *::boost::unit_test::timeout (10))
{
  fhg::util::scoped_boost_asio_io_service_with_threads io_service (0);

  bool called (false);

  io_service.post ([&called] { called = true; });

  //! \note enough for a different thread to jump in and execute the
  //! posted function, if there was one
  std::this_thread::sleep_for (std::chrono::milliseconds (100));

  BOOST_REQUIRE (!called);

  io_service.run_one();

  BOOST_REQUIRE (called);
}

BOOST_AUTO_TEST_CASE (normal_procedure, *::boost::unit_test::timeout (10))
{
  fhg::util::scoped_boost_asio_io_service_with_threads io_service (1);

  fhg::util::latch called (1);

  io_service.post ([&called] { called.count_down(); });

  called.wait();
}

BOOST_AUTO_TEST_CASE ( actually_starts_multiple_threads_if_asked_to
                     , *::boost::unit_test::timeout (10)
                     )
{
  std::size_t const thread_count (10);

  fhg::util::scoped_boost_asio_io_service_with_threads
    io_service (thread_count);

  fhg::util::latch called (thread_count);
  fhg::util::latch release (1);

  auto&& fun ( [&called, &release]
               {
                 called.count_down();
                 release.wait();
               }
             );

  for (std::size_t i (0); i < thread_count; ++i)
  {
    io_service.post (fun);
  }

  called.wait();
  release.count_down();
}

BOOST_AUTO_TEST_CASE ( deferred_with_zero_does_not_make_sense_due_to_api
                     , *::boost::unit_test::timeout (10)
                     )
{
  fhg::util::testing::require_exception
    ( []
      {
        fhg::util::scoped_boost_asio_io_service_with_threads_and_deferred_startup io_service (0);
      }
    , std::invalid_argument
        ( "count shall be > 0 due to start_in_threads_and_current_thread()"
          " invoking at least one run() in the current thread."
        )
    );
}

BOOST_AUTO_TEST_CASE ( deferred_with_one_thread_runs_in_main_thread
                     , *::boost::unit_test::timeout (10)
                     )
{
  fhg::util::scoped_boost_asio_io_service_with_threads_and_deferred_startup
    io_service (1);

  auto const main_thread (std::this_thread::get_id());
  std::thread::id seen_thread;

  io_service.post
    ( [&]
      {
        seen_thread = std::this_thread::get_id();
        io_service.stop();
      }
    );

  io_service.post_fork_parent();
  io_service.start_in_threads_and_current_thread();

  BOOST_REQUIRE_EQUAL (main_thread, seen_thread);
}

BOOST_AUTO_TEST_CASE ( deferred_runs_in_main_thread_and_n_minus_one_more
                     , *::boost::unit_test::timeout (10)
                     )
{
  std::size_t const thread_count (10);
  fhg::util::scoped_boost_asio_io_service_with_threads_and_deferred_startup
    io_service (thread_count);

  auto const main_thread (std::this_thread::get_id());

  fhg::util::latch called (thread_count - 1);
  fhg::util::latch release (1);

  auto&& fun
    ( [&io_service, &main_thread, &called, &release]
      {
        if (main_thread == std::this_thread::get_id())
        {
          called.wait();
          release.count_down();
          io_service.stop();
        }
        else
        {
          called.count_down();
          release.wait();
        }
      }
    );

  for (std::size_t i (0); i < thread_count; ++i)
  {
    io_service.post (fun);
  }

  io_service.post_fork_parent();
  io_service.start_in_threads_and_current_thread();
}
