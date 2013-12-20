#define BOOST_TEST_MODULE GspcRifManagerTests
#include <boost/test/unit_test.hpp>

#include <errno.h>
#include <signal.h>

#include <iostream>

#include <boost/foreach.hpp>

#include <gspc/rif/manager.hpp>
#include <gspc/rif/proc_info.hpp>

#include <fhg/util/boost/test/printer/vector.hpp>
#include <fhg/util/boost/test/printer/map.hpp>

struct F
{
  F ()
  {
    signal (SIGCHLD, SIG_DFL);
  }
};

struct handler_t : gspc::rif::process_handler_t
{
  void onStateChange (gspc::rif::proc_t p, gspc::rif::process_state_t s)
  {
    proc = p;
    state = s;
  }

  gspc::rif::proc_t proc;
  gspc::rif::process_state_t state;
};

BOOST_FIXTURE_TEST_SUITE( suite, F )

BOOST_AUTO_TEST_CASE (test_start_stop)
{
  static const size_t NUM_IERATIONS = 100;

  gspc::rif::manager_t manager;

  for (size_t iter = 0 ; iter < NUM_IERATIONS ; ++iter)
  {
    manager.start ();
    manager.stop ();
  }
}

BOOST_AUTO_TEST_CASE (test_exec)
{
  int rc;
  int status;
  gspc::rif::proc_t p;
  handler_t handler;
  gspc::rif::manager_t manager;
  manager.start ();
  manager.register_handler (&handler);

  gspc::rif::argv_t argv;

  argv.push_back ("/bin/echo");
  argv.push_back ("hello");
  argv.push_back ("world");

  p = manager.exec (argv);
  BOOST_REQUIRE_GT (p, 0);
  BOOST_REQUIRE_EQUAL (handler.proc, p);
  BOOST_REQUIRE_EQUAL (handler.state, gspc::rif::PROCESS_STARTED);

  rc = manager.wait (p, &status);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE (WIFEXITED (status));
  BOOST_REQUIRE_EQUAL (WEXITSTATUS (status), 0);
  BOOST_REQUIRE_EQUAL (handler.proc, p);
  BOOST_REQUIRE_EQUAL (handler.state, gspc::rif::PROCESS_TERMINATED);

  manager.stop ();
}

BOOST_AUTO_TEST_CASE (test_echo_no_newline)
{
  int rc;
  int status;
  gspc::rif::proc_t p;
  gspc::rif::manager_t manager;
  manager.start ();

  gspc::rif::argv_t argv;

  argv.push_back ("/bin/echo");
  argv.push_back ("-n");
  argv.push_back ("hello");
  argv.push_back ("world");

  p = manager.exec (argv);
  BOOST_REQUIRE_GT (p, 0);

  rc = manager.wait (p, &status);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE (WIFEXITED (status));
  BOOST_REQUIRE_EQUAL (WEXITSTATUS (status), 0);

  manager.stop ();
}

BOOST_AUTO_TEST_CASE (test_no_output)
{
  int rc;
  int status;
  gspc::rif::proc_t p;
  gspc::rif::manager_t manager;
  manager.start ();

  gspc::rif::argv_t argv;

  argv.push_back ("/bin/true");

  p = manager.exec (argv);
  BOOST_REQUIRE_GT (p, 0);

  rc = manager.wait (p, &status);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE (WIFEXITED (status));
  BOOST_REQUIRE_EQUAL (WEXITSTATUS (status), 0);

  manager.stop ();
}

BOOST_AUTO_TEST_CASE (test_cat)
{
  int rc;
  int status;
  gspc::rif::proc_t p;
  gspc::rif::manager_t manager;
  manager.start ();

  gspc::rif::argv_t argv;

  argv.push_back ("/bin/cat");
  argv.push_back ("/proc/cpuinfo");

  p = manager.exec (argv);
  BOOST_REQUIRE_GT (p, 0);

  rc = manager.wait (p, &status);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE (WIFEXITED (status));
  BOOST_REQUIRE_EQUAL (WEXITSTATUS (status), 0);

  manager.stop ();
}

BOOST_AUTO_TEST_CASE (test_many_processes)
{
  static const std::size_t NUM_PROCS = 128;

  int rc;
  int status;
  gspc::rif::proc_t p;
  gspc::rif::manager_t manager;
  manager.start ();

  for (std::size_t i = 0 ; i < NUM_PROCS ; ++i)
  {
    gspc::rif::argv_t argv;

    argv.push_back ("/bin/true");

    p = manager.exec (argv);
    BOOST_REQUIRE_GT (p, 0);

    rc = manager.wait (p, &status);
    BOOST_REQUIRE_EQUAL (rc, 0);
    BOOST_REQUIRE (WIFEXITED (status));
    BOOST_REQUIRE_EQUAL (WEXITSTATUS (status), 0);

    rc = manager.remove (p);
    BOOST_REQUIRE_EQUAL (rc, 0);
  }

  manager.stop ();
}

BOOST_AUTO_TEST_CASE (test_sigterm)
{
  int rc;
  int status;
  gspc::rif::proc_t p;
  gspc::rif::manager_t manager;
  manager.start ();

  gspc::rif::argv_t argv;

  argv.push_back ("/bin/cat");

  p = manager.exec (argv);
  BOOST_REQUIRE_GT (p, 0);

  rc = manager.kill (p, SIGTERM);
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = manager.wait (p, &status);
  BOOST_REQUIRE_EQUAL (rc, 0);

  BOOST_REQUIRE (WIFSIGNALED (status));
  BOOST_REQUIRE_EQUAL (WTERMSIG (status), SIGTERM);

  rc = manager.remove (p);
  BOOST_REQUIRE_EQUAL (rc, 0);

  manager.stop ();
}

BOOST_AUTO_TEST_CASE (test_parallel_sleeps)
{
  const std::size_t NUM_PROCS = 128;

  int rc;
  gspc::rif::manager_t manager;
  gspc::rif::proc_list_t ids;
  manager.start ();

  for (size_t i = 0 ; i < NUM_PROCS ; ++i)
  {
    gspc::rif::argv_t argv;
    argv.push_back ("/bin/sleep");
    argv.push_back ("5");

    gspc::rif::proc_t p (manager.exec (argv));
    if (p > 0)
    {
      ids.push_back (p);
    }
    else
    {
      std::cerr << "failed to exec: " << p << ": " << strerror (-p) << std::endl;
    }
  }

  std::cerr << ids.size () << " processes started" << std::endl;

  BOOST_REQUIRE_EQUAL (manager.processes ().size (), ids.size ());

  BOOST_FOREACH (gspc::rif::proc_t p, ids)
  {
    int status;
    rc = manager.wait (p, &status);
    BOOST_REQUIRE_EQUAL (rc, 0);
    BOOST_REQUIRE (WIFEXITED (status));
    BOOST_REQUIRE_EQUAL (WEXITSTATUS (status), 0);

    rc = manager.remove (p);
    BOOST_REQUIRE_EQUAL (rc, 0);
  }

  std::cerr << ids.size () << " processes finished" << std::endl;

  BOOST_REQUIRE_EQUAL (manager.processes ().size (), 0);

  manager.stop ();
}

BOOST_AUTO_TEST_CASE (test_proc_info)
{
  int rc;
  int status;
  gspc::rif::proc_t p;
  gspc::rif::manager_t manager;
  gspc::rif::proc_info_t info;

  manager.start ();

  gspc::rif::argv_t argv;

  argv.push_back ("/bin/cat");

  p = manager.exec (argv, gspc::rif::env_t ());
  BOOST_REQUIRE_GT (p, 0);

  rc = manager.proc_info (p, info);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (info.id (), p);
  BOOST_REQUIRE_GT (info.pid (), 1);
  BOOST_REQUIRE_EQUAL (info.argv (), argv);
  BOOST_REQUIRE (info.env ().empty ());
  BOOST_REQUIRE (not info.status ());

  rc = manager.kill (p, SIGTERM);
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = manager.wait (p, &status);
  BOOST_REQUIRE_EQUAL (rc, 0);

  BOOST_REQUIRE (WIFSIGNALED (status));
  BOOST_REQUIRE_EQUAL (WTERMSIG (status), SIGTERM);

  rc = manager.proc_info (p, info);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (info.id (), p);
  BOOST_REQUIRE_EQUAL (info.pid (), -1);
  BOOST_REQUIRE_EQUAL (info.argv (), argv);
  BOOST_REQUIRE (info.env ().empty ());
  BOOST_REQUIRE (info.status ());

  rc = manager.remove (p);
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = manager.proc_info (p, info);
  BOOST_REQUIRE_EQUAL (rc, -ESRCH);

  manager.stop ();
}

BOOST_AUTO_TEST_CASE (test_communicate)
{
  int rc;
  int status;
  gspc::rif::proc_t p;
  gspc::rif::manager_t manager;
  char buf [4096];
  boost::system::error_code ec;

  manager.start ();

  gspc::rif::argv_t argv;

  argv.push_back ("/bin/cat");

  p = manager.exec (argv);
  BOOST_REQUIRE_GT (p, 0);

  const std::string text = "hello world!";

  rc = manager.write (p, STDIN_FILENO, text.c_str (), text.size (), ec);
  BOOST_REQUIRE_GT (rc, 0);
  BOOST_REQUIRE_EQUAL ((std::size_t)rc, text.size ());

  do
  {
    rc = manager.read (p, STDOUT_FILENO, buf, sizeof(buf), ec);
    if (rc <= 0)
      usleep (500);
  }
  while (rc <= 0);
  BOOST_REQUIRE_GT (rc, 0);
  BOOST_REQUIRE_EQUAL ((std::size_t)rc, text.size ());

  rc = manager.read (p, STDOUT_FILENO, buf, sizeof(buf), ec);
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = manager.kill (p, SIGTERM);
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = manager.wait (p, &status);
  BOOST_REQUIRE_EQUAL (rc, 0);

  BOOST_REQUIRE (WIFSIGNALED (status));
  BOOST_REQUIRE_EQUAL (WTERMSIG (status), SIGTERM);

  rc = manager.remove (p);
  BOOST_REQUIRE_EQUAL (rc, 0);

  manager.stop ();
}

BOOST_AUTO_TEST_CASE (test_async_handler)
{
  int rc;
  int status;
  gspc::rif::proc_t p;
  handler_t handler;
  gspc::rif::manager_t manager;
  manager.start ();
  manager.register_handler (&handler);

  gspc::rif::argv_t argv;

  argv.push_back ("/bin/sleep");
  argv.push_back ("1");

  p = manager.exec (argv);
  BOOST_REQUIRE_GT (p, 0);
  BOOST_REQUIRE_EQUAL (handler.proc, p);
  BOOST_REQUIRE_EQUAL (handler.state, gspc::rif::PROCESS_STARTED);

  sleep (2); // give enough time to get an async handler callback

  BOOST_REQUIRE_EQUAL (handler.proc, p);
  BOOST_REQUIRE_EQUAL (handler.state, gspc::rif::PROCESS_TERMINATED);

  rc = manager.wait (p, &status);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE (WIFEXITED (status));
  BOOST_REQUIRE_EQUAL (WEXITSTATUS (status), 0);
  BOOST_REQUIRE_EQUAL (handler.proc, p);
  BOOST_REQUIRE_EQUAL (handler.state, gspc::rif::PROCESS_TERMINATED);

  manager.stop ();
}

BOOST_AUTO_TEST_CASE (test_wait_twice)
{
  int rc;
  int status;
  gspc::rif::proc_t p;
  handler_t handler;
  gspc::rif::manager_t manager;
  manager.start ();
  manager.register_handler (&handler);

  gspc::rif::argv_t argv;

  argv.push_back ("/bin/cat");

  p = manager.exec (argv);
  BOOST_REQUIRE_GT (p, 0);
  BOOST_REQUIRE_EQUAL (handler.proc, p);
  BOOST_REQUIRE_EQUAL (handler.state, gspc::rif::PROCESS_STARTED);

  manager.kill (p, SIGTERM);

  for (int i = 0 ; i < 2 ; ++i)
  {
    rc = manager.wait (p, &status);

    BOOST_REQUIRE_EQUAL (rc, 0);
    BOOST_REQUIRE (WIFSIGNALED (status));
    BOOST_REQUIRE_EQUAL (WTERMSIG (status), SIGTERM);
    BOOST_REQUIRE_EQUAL (handler.proc, p);
    BOOST_REQUIRE_EQUAL (handler.state, gspc::rif::PROCESS_TERMINATED);
  }

  manager.stop ();
}

BOOST_AUTO_TEST_SUITE_END()
