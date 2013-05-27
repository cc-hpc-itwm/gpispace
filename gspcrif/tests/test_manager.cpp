#define BOOST_TEST_MODULE GspcRifManagerTests
#include <boost/test/unit_test.hpp>

#include <errno.h>
#include <signal.h>
#include <gspc/rif/manager.hpp>

struct F
{
  F ()
  {
    signal (SIGCHLD, SIG_DFL);
  }
};

BOOST_FIXTURE_TEST_SUITE( suite, F )

BOOST_AUTO_TEST_CASE (test_start_stop)
{
  static const int NUM_IERATIONS = 100;

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
  gspc::rif::manager_t manager;
  manager.start ();

  gspc::rif::argv_t argv;

  argv.push_back ("/bin/echo");
  argv.push_back ("hello");
  argv.push_back ("world");

  p = manager.exec (argv);
  BOOST_REQUIRE (p > 0);
  std::cerr << "process created: " << p << std::endl;

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
  BOOST_REQUIRE (p > 0);
  std::cerr << "process created: " << p << std::endl;

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
  BOOST_REQUIRE (p > 0);
  std::cerr << "process created: " << p << std::endl;

  rc = manager.wait (p, &status);
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE (WIFEXITED (status));
  BOOST_REQUIRE_EQUAL (WEXITSTATUS (status), 0);

  manager.stop ();
}

BOOST_AUTO_TEST_CASE (test_many_processes)
{
  static const std::size_t NUM_PROCS = 1024;

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
    BOOST_REQUIRE (p > 0);

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
  BOOST_REQUIRE (p > 0);

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

BOOST_AUTO_TEST_SUITE_END()
