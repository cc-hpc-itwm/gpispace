// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <drts/worker/context.hpp>
#include <drts/worker/context_impl.hpp>

#include <util-generic/syscall.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/random/string.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <chrono>
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

namespace
{
  //! \todo factor out common functionality with process::execute
  struct channel_from_child_to_parent
  {
    channel_from_child_to_parent()
    {
      fhg::util::syscall::pipe (_fds.both);
    }

    template<typename T>
      void write (T&& x) const
    {
      BOOST_REQUIRE_EQUAL
        (sizeof (x), fhg::util::syscall::write (_fds.fd.write, &x, sizeof (x)));
    }
    template<typename T>
      T read() const
    {
      T x;

      BOOST_REQUIRE_EQUAL
        (sizeof (x), fhg::util::syscall::read (_fds.fd.read, &x, sizeof (x)));

      return x;
    }

  private:
    struct fds
    {
      int read;
      int write;
    };
    union
    {
      int both[2];
      struct fds fd;
    } _fds;
  };

  struct context_fixture
  {
    context_fixture()
      : logger()
      , context
        ( drts::worker::context_constructor
          (fhg::util::testing::random_string(), {}, logger)
        )
    {
      // \todo Remove once we properly flush on forks (issue #751).
      std::cout.flush();
      std::cerr.flush();
      std::clog.flush();
    }

    fhg::logging::stream_emitter logger;
    drts::worker::context context;
  };

  void on_cancel_unexpected()
  {
    BOOST_FAIL ("unexpected on_cancel()");
  }
  void on_signal_unexpected (int signal)
  {
    BOOST_FAIL ("unexpected on_signal (" + std::to_string (signal) + ")");
  }
  void on_exit_unexpected (int exit_code)
  {
    BOOST_FAIL ("unexpected on_exit (" + std::to_string (exit_code) + ")");
  }
}

BOOST_FIXTURE_TEST_CASE
  (execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN_runs_function_and_calls_on_exit, context_fixture)
{
  int const r {fhg::util::testing::random<int>()()};

  bool exited {false};

  channel_from_child_to_parent channel;

  context.execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
    ( [r, &channel]()
      {
        channel.write (r);
      }
    , &on_cancel_unexpected
    , &on_signal_unexpected
    , [&exited] (int exit_code)
      {
        BOOST_REQUIRE_EQUAL (exit_code, 0);

        exited = true;
      }
    );

  BOOST_REQUIRE_EQUAL (r, channel.read<int>());

  BOOST_REQUIRE (exited);
}

BOOST_FIXTURE_TEST_CASE
  (execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN_calls_on_cancel, context_fixture)
{
  bool cancelled {false};
  bool signaled {false};
  bool exited {false};

  std::thread execution
    ( [&]
      {
        std::this_thread::sleep_for
          ( std::chrono::milliseconds
            (fhg::util::testing::random<int>()() % 500)
          );

        context.execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
          ( []()
            {
              while (true) {}
            }
          , [&cancelled] ()
            {
              cancelled = true;
            }
          , [&] (int) { signaled = true; }
          , [&] (int) { exited = true; }
          );
      }
    );

  context.module_call_do_cancel();

  execution.join();

  BOOST_REQUIRE (cancelled);
  BOOST_REQUIRE (!signaled);
  BOOST_REQUIRE (!exited);
}

BOOST_FIXTURE_TEST_CASE
  (execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN_calls_on_signal, context_fixture)
{
  bool signalled {false};

  context.execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
    ( []()
      {
        fhg::util::syscall::kill (fhg::util::syscall::getpid(), SIGUSR1);
      }
    , &on_cancel_unexpected
    , [&signalled] (int signal)
      {
        BOOST_REQUIRE_EQUAL (signal, SIGUSR1);

        signalled = true;
      }
    , &on_exit_unexpected
    );

  BOOST_REQUIRE (signalled);
}

BOOST_FIXTURE_TEST_CASE
  (execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN_calls_SIGTERM_blocked, context_fixture)
{
  bool exited {false};

  context.execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
    ( [] ()
      {
        fhg::util::syscall::kill (fhg::util::syscall::getpid(), SIGTERM);
      }
    , &on_cancel_unexpected
    , &on_signal_unexpected
    , [&exited] (int exit_code)
      {
        BOOST_REQUIRE_EQUAL (exit_code, 0);

        exited = true;
      }
    );

  BOOST_REQUIRE (exited);
}

BOOST_FIXTURE_TEST_CASE
  (execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN_calls_SIGINT_blocked, context_fixture)
{
  bool exited {false};

  context.execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
    ( [] ()
      {
        fhg::util::syscall::kill (fhg::util::syscall::getpid(), SIGINT);
      }
    , &on_cancel_unexpected
    , &on_signal_unexpected
    , [&exited] (int exit_code)
      {
        BOOST_REQUIRE_EQUAL (exit_code, 0);

        exited = true;
      }
    );

  BOOST_REQUIRE (exited);
}

BOOST_FIXTURE_TEST_CASE (execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN_with_throw, context_fixture)
{
  std::runtime_error const exception {fhg::util::testing::random_string()};

  fhg::util::testing::require_exception
    ( [this, &exception]()
      {
        context.execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
          ( [&exception]()
            {
              throw exception;
            }
          , &on_cancel_unexpected
          , &on_signal_unexpected
          , &on_exit_unexpected
          );
      }
    , exception
    );
}

BOOST_FIXTURE_TEST_CASE (execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN_fun_exit, context_fixture)
{
  for (int ec (0); ec < 256; ++ec)
  {
    bool exited {false};

    context.execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
      ( [&ec]()
        {
          exit (ec);
        }
      , &on_cancel_unexpected
      , &on_signal_unexpected
      , [&ec, &exited] (int exit_code)
        {
          BOOST_REQUIRE_EQUAL (exit_code, ec);

          exited = true;
        }
      );

    BOOST_REQUIRE (exited);
  }
}

BOOST_FIXTURE_TEST_CASE
  (execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN_simpler_unexpected_signal, context_fixture)
{
  fhg::util::testing::require_exception
    ( [this]()
      {
        context.execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
          ( []()
            {
              fhg::util::syscall::kill (fhg::util::syscall::getpid(), SIGUSR1);
            }
          , &on_cancel_unexpected
          );
      }
    , std::logic_error
        ("Unexpected on_signal (" + std::to_string (SIGUSR1) + ")")
    );
}

BOOST_FIXTURE_TEST_CASE
  (execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN_simpler_unexpected_exit, context_fixture)
{
  for (int ec (0); ec < 256; ++ec)
  {
    fhg::util::testing::require_exception
      ( [this, ec]()
      {
        context.execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
          ( [ec] ()
            {
              exit (ec);
            }
          , &on_cancel_unexpected
          );
      }
      , std::logic_error ("Unexpected on_exit (" + std::to_string (ec) + ")")
      );
  }
}

BOOST_FIXTURE_TEST_CASE
  (execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN_simpler_cancelled, context_fixture)
{
  auto future
    ( std::async ( std::launch::async
                 , [&]
                   {
                     context.execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
                       ( []
                         {
                           while (true) {}
                         }
                       );
                   }
                 )
    );

  context.module_call_do_cancel();

  fhg::util::testing::require_exception
    ( [&]
      {
        future.get();
      }
    , drts::worker::context::cancelled()
    );
}

BOOST_FIXTURE_TEST_CASE
  ( execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN_can_be_called_more_often_than_max_open_files
  , context_fixture
  )
{
  long const maximum_open_files (fhg::util::syscall::sysconf (_SC_OPEN_MAX));

  for (long i (0); i < 2 * maximum_open_files; ++i)
  {
    context.execute_and_kill_on_cancel_DO_NOT_OUTPUT_TO_STANDARD_STREAMS_FROM_WITHIN
      ( [](){}
      , &on_cancel_unexpected
      , &on_signal_unexpected
      , [] (int exit_code) { BOOST_REQUIRE_EQUAL (exit_code, 0); }
      );
  }
}
