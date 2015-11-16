
#define BOOST_TEST_MODULE drts_context
#include <boost/test/unit_test.hpp>

#include <drts/worker/context.hpp>
#include <drts/worker/context_impl.hpp>

#include <util-generic/syscall.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/random_string.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <thread>
#include <chrono>

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
        (sizeof (x), fhg::util::syscall::write (_fds.write, &x, sizeof (x)));
    }
    template<typename T>
      T read() const
    {
      T x;

      BOOST_REQUIRE_EQUAL
        (sizeof (x), fhg::util::syscall::read (_fds.read, &x, sizeof (x)));

      return x;
    }

  private:
    union
    {
      int both[2];
      struct
      {
        int read;
        int write;
      };
    } _fds;
  };

  struct cf
  {
    cf()
      : logger()
      , context
        ( drts::worker::context_constructor
          (fhg::util::testing::random_string(), {}, logger)
        )
    {}

    fhg::log::Logger logger;
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
  (execute_and_kill_on_cancel_runs_function_and_calls_on_exit, cf)
{
  int const r {fhg::util::testing::random<int>()()};

  bool exited {false};

  channel_from_child_to_parent channel;

  context.execute_and_kill_on_cancel
    ( [r, &channel, &exited]()
      {
        channel.write (r);
      }
    , &on_cancel_unexpected
    , &on_signal_unexpected
    , [r, &exited] (int exit_code)
      {
        BOOST_REQUIRE_EQUAL (exit_code, 0);

        exited = true;
      }
    );

  BOOST_REQUIRE_EQUAL (r, channel.read<int>());

  BOOST_REQUIRE (exited);
}

BOOST_FIXTURE_TEST_CASE (execute_and_kill_on_cancel_calls_on_cancel, cf)
{
  bool cancelled {false};

  std::thread execution
    ( [this, &cancelled]()
      {
        std::this_thread::sleep_for
          ( std::chrono::milliseconds
            (fhg::util::testing::random<int>()() % 500)
          );

        context.execute_and_kill_on_cancel
          ( []()
            {
              while (1) {}
            }
          , [&cancelled] ()
            {
              cancelled = true;
            }
          , &on_signal_unexpected
          , &on_exit_unexpected
          );
      }
    );

  context.module_call_do_cancel();

  execution.join();

  BOOST_REQUIRE (cancelled);
}

BOOST_FIXTURE_TEST_CASE (execute_and_kill_on_cancel_calls_on_signal, cf)
{
  bool signalled {false};

  context.execute_and_kill_on_cancel
    ( [&signalled] ()
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

BOOST_FIXTURE_TEST_CASE (execute_and_kill_on_cancel_with_throw, cf)
{
  std::runtime_error const exception {fhg::util::testing::random_string()};

  fhg::util::testing::require_exception
    ( [this, &exception]()
      {
        context.execute_and_kill_on_cancel
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

BOOST_FIXTURE_TEST_CASE (execute_and_kill_on_cancel_fun_exit, cf)
{
  for (int ec (0); ec < 256; ++ec)
  {
    bool exited {false};

    context.execute_and_kill_on_cancel
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
