#define BOOST_TEST_MODULE GspcRifProcessTests
#include <boost/test/unit_test.hpp>

#include <errno.h>
#include <signal.h>
#include <gspc/rif/process.hpp>

struct F
{
  F ()
  {
    // do not let boost handle sigchild
    signal (SIGCHLD, SIG_DFL);
  }
};

BOOST_FIXTURE_TEST_SUITE( suite, F )

BOOST_AUTO_TEST_CASE (test_exec_write_read_kill)
{
  gspc::rif::argv_t argv;
  gspc::rif::env_t env;
  int rc;
  char buf [4096];

  memset (buf, 0, sizeof(buf));

  argv.push_back ("/bin/cat");

  gspc::rif::process_t proc (0, argv.front (), argv, env);

  rc = proc.fork_and_exec ();

  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE (proc.pid () > 0);

  std::string text ("hello world\n");
  BOOST_REQUIRE_EQUAL ( proc.write (text.c_str (), text.size ())
                      , (ssize_t)(text.size ())
                      );

  BOOST_REQUIRE_EQUAL ( proc.read (buf, sizeof(buf))
                      , (ssize_t)(text.size ())
                      );

  BOOST_REQUIRE_EQUAL (buf, "hello world\n");

  rc = proc.try_waitpid ();
  BOOST_REQUIRE_EQUAL (rc, -EBUSY);

  rc = proc.kill (SIGTERM);
  BOOST_REQUIRE_EQUAL (rc, 0);

  rc = proc.waitpid ();
  BOOST_REQUIRE_EQUAL (rc, 0);

  int status = *proc.status ();
  BOOST_REQUIRE (WIFSIGNALED (status));
  BOOST_REQUIRE_EQUAL (WTERMSIG (status), SIGTERM);
}

BOOST_AUTO_TEST_CASE (test_no_such_file_or_directory)
{
  gspc::rif::argv_t argv;
  gspc::rif::env_t env;
  int rc;
  char buf [4096];

  memset (buf, 0, sizeof(buf));

  argv.push_back ("/nosuchfile.9215179f1ff34960976abef7de742076");

  gspc::rif::process_t proc (0, argv.front (), argv, env);

  rc = proc.fork_and_exec ();

  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE (proc.pid () > 0);

  rc = proc.waitpid ();
  BOOST_REQUIRE_EQUAL (rc, 0);

  int status = *proc.status ();
  BOOST_REQUIRE (WIFEXITED (status));
  BOOST_REQUIRE_EQUAL (WEXITSTATUS (status), 127);
}

BOOST_AUTO_TEST_CASE (test_permission_denied)
{
  gspc::rif::argv_t argv;
  gspc::rif::env_t env;
  int rc;
  char buf [4096];

  memset (buf, 0, sizeof(buf));

  argv.push_back ("/root/this.must.fail");

  gspc::rif::process_t proc (0, argv.front (), argv, env);

  rc = proc.fork_and_exec ();

  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE (proc.pid () > 0);

  rc = proc.waitpid ();
  BOOST_REQUIRE_EQUAL (rc, 0);

  int status = *proc.status ();
  BOOST_REQUIRE (WIFEXITED (status));
  BOOST_REQUIRE_EQUAL (WEXITSTATUS (status), 126);
}

BOOST_AUTO_TEST_CASE (test_empty_argv)
{
  gspc::rif::argv_t argv;
  gspc::rif::env_t env;
  int rc;
  char buf [4096];

  memset (buf, 0, sizeof(buf));

  gspc::rif::process_t proc (0, "", argv, env);

  rc = proc.fork_and_exec ();
  BOOST_REQUIRE_EQUAL (rc, -EINVAL);
}

BOOST_AUTO_TEST_CASE (test_environment)
{
  gspc::rif::argv_t argv;
  gspc::rif::env_t env;
  int rc;
  char buf [4096];

  memset (buf, 0, sizeof(buf));
  env ["foo"] = "bar";

  argv.push_back ("/usr/bin/env");
  gspc::rif::process_t proc (0, argv.front (), argv, env);

  rc = proc.fork_and_exec ();

  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE (proc.pid () > 0);

  proc.read (buf, sizeof(buf));

  rc = proc.waitpid ();
  BOOST_REQUIRE_EQUAL (rc, 0);

  int status = *proc.status ();
  BOOST_REQUIRE (WIFEXITED (status));
  BOOST_REQUIRE_EQUAL (WEXITSTATUS (status), 0);

  BOOST_REQUIRE_EQUAL (buf, "foo=bar\n");
}

BOOST_AUTO_TEST_SUITE_END()
