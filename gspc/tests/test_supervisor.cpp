#define BOOST_TEST_MODULE GspcRifSupervisor
#include <boost/test/unit_test.hpp>

#include <errno.h>
#include <signal.h>

#include <fhg/util/thread/semaphore.hpp>
#include <gspc/rif/manager.hpp>
#include <gspc/rif/supervisor.hpp>
#include <gspc/rif/util.hpp>

#include <boost/foreach.hpp>

struct F
{
  F ()
  {
    // do not let boost handle sigchild
    signal (SIGCHLD, SIG_DFL);
  }
};

BOOST_FIXTURE_TEST_SUITE (suite, F)

static void s_child_failed ( gspc::rif::supervisor_t *sup
                           , bool *done
                           , gspc::rif::supervisor_t::child_info_t const &info
                           )
{
  std::cout << "child (" << info.descriptor.name << ") permanently failed:" << std::endl;

  BOOST_FOREACH ( gspc::rif::supervisor_t::error_info_t const & error
                , info.errors
                )
  {
    std::cout << "\tstatus: " << gspc::rif::make_exit_code (error.status) << std::endl
              << "\t\twhen: " << error.tstamp << std::endl
              << "\t\tout: " <<  error.stdout << std::endl
              << "\t\terr: " <<  error.stderr << std::endl
      ;
  }

  *done = true;
}

static void s_child_terminated ( gspc::rif::supervisor_t *sup
                               , gspc::rif::supervisor_t::child_info_t const &info
                               )
{
  std::cout << "child (" << info.descriptor.name << ") terminated" << std::endl;
}

static void s_child_started ( gspc::rif::supervisor_t *sup
                            , gspc::rif::supervisor_t::child_info_t const &info
                            )
{
  std::cout << "child (" << info.descriptor.name << ") started" << std::endl;
}

static void s_wakeup (fhg::thread::semaphore *sem)
{
  sem->V ();
}

BOOST_AUTO_TEST_CASE (test_supervise_basics)
{
  gspc::rif::argv_t argv;
  gspc::rif::env_t env;
  int rc;
  bool done;
  char buf [4096];
  fhg::thread::semaphore sem (0);

  memset (buf, 0, sizeof(buf));

  argv.push_back ("/bin/cat");

  gspc::rif::manager_t mgr;
  mgr.start ();

  gspc::rif::supervisor_t sup (mgr, 3, 60);
  sup.onChildFailed.connect
    (boost::bind ( s_child_failed
                 , &sup
                 , &done
                 , _1
                 )
    );
  sup.onChildFailed.connect
    (boost::bind ( s_wakeup
                 , &sem
                 )
    );
  sup.onChildTerminated.connect
    (boost::bind ( s_child_terminated
                 , &sup
                 , _1
                 )
    );
  sup.onChildTerminated.connect
    (boost::bind ( s_wakeup
                 , &sem
                 )
    );
  sup.onChildStarted.connect
    (boost::bind ( s_child_started
                 , &sup
                 , _1
                 )
    );
  sup.onChildStarted.connect
    (boost::bind ( s_wakeup
                 , &sem
                 )
    );

  done = false;

  sup.start ();

  rc = sup.add_child
    (gspc::rif::child_descriptor_t ( "cat"
                                   , argv
                                   , env
                                   , gspc::rif::child_descriptor_t::RESTART_ALWAYS
                                   , gspc::rif::child_descriptor_t::SHUTDOWN_KILL
                                   , 0
                                   )
    );

  BOOST_REQUIRE_EQUAL (rc, 0);

  sem.P ();

  rc = sup.terminate_child ("cat");

  BOOST_REQUIRE_EQUAL (rc, 0);

  sem.P ();

  sup.remove_child ("cat");

  sup.stop ();
}

BOOST_AUTO_TEST_CASE (test_supervise_cat)
{
  gspc::rif::argv_t argv;
  gspc::rif::env_t env;
  int rc;
  bool done;
  char buf [4096];
  fhg::thread::semaphore sem (0);

  memset (buf, 0, sizeof(buf));

  argv.push_back ("/bin/cat");

  gspc::rif::manager_t mgr;
  mgr.start ();

  gspc::rif::supervisor_t sup (mgr, 3, 60);
  sup.onChildFailed.connect
    (boost::bind ( s_child_failed
                 , &sup
                 , &done
                 , _1
                 )
    );
  sup.onChildTerminated.connect
    (boost::bind ( s_child_terminated
                 , &sup
                 , _1
                 )
    );
  sup.onChildStarted.connect
    (boost::bind ( s_child_started
                 , &sup
                 , _1
                 )
    );
  sup.onSupervisorStopped.connect
    (boost::bind (s_wakeup, &sem));

  done = false;

  sup.start ();

  rc = sup.add_child
    (gspc::rif::child_descriptor_t ( "cat"
                                   , argv
                                   , env
                                   , gspc::rif::child_descriptor_t::RESTART_ALWAYS
                                   , gspc::rif::child_descriptor_t::SHUTDOWN_KILL
                                   , 0
                                   )
    );

  BOOST_REQUIRE_EQUAL (rc, 0);

  while (not done)
  {
    gspc::rif::supervisor_t::child_info_t info =
      sup.get_child_info ("cat");

    mgr.kill (info.proc, SIGKILL);
    sleep (1);
  }

  sup.stop ();
  sem.P ();
}

BOOST_AUTO_TEST_CASE (test_supervise_sleep)
{
  gspc::rif::argv_t argv;
  gspc::rif::env_t env;
  int rc;
  char buf [4096];
  bool done;
  fhg::thread::semaphore sem (0);

  memset (buf, 0, sizeof(buf));

  argv.push_back ("/bin/sleep");
  argv.push_back ("2");

  gspc::rif::manager_t mgr;
  mgr.start ();

  gspc::rif::supervisor_t sup (mgr, 1, 4);
  sup.onChildFailed.connect
    (boost::bind ( s_child_failed
                 , &sup
                 , &done
                 , _1
                 )
    );
  sup.onChildFailed.connect
    (boost::bind ( s_wakeup
                 , &sem
                 )
    );
  sup.onChildTerminated.connect
    (boost::bind ( s_child_terminated
                 , &sup
                 , _1
                 )
    );
  sup.onChildStarted.connect
    (boost::bind ( s_child_started
                 , &sup
                 , _1
                 )
    );

  sup.start ();

  rc = sup.add_child
    (gspc::rif::child_descriptor_t ( "sleep"
                                   , argv
                                   , env
                                   , gspc::rif::child_descriptor_t::RESTART_ALWAYS
                                   , gspc::rif::child_descriptor_t::SHUTDOWN_KILL
                                   , 0
                                   )
    );

  BOOST_REQUIRE_EQUAL (rc, 0);

  sem.P ();
}

BOOST_AUTO_TEST_CASE (test_supervise_failing_ls)
{
  gspc::rif::argv_t argv;
  gspc::rif::env_t env;
  int rc;
  char buf [4096];
  bool done;
  fhg::thread::semaphore sem (0);

  memset (buf, 0, sizeof(buf));

  argv.push_back ("/bin/ls");
  argv.push_back ("/nosuchdirectory");

  gspc::rif::manager_t mgr;
  mgr.start ();

  gspc::rif::supervisor_t sup (mgr, 2, 4);
  sup.onChildFailed.connect
    (boost::bind ( s_child_failed
                 , &sup
                 , &done
                 , _1
                 )
    );
  sup.onChildFailed.connect
    (boost::bind ( s_wakeup
                 , &sem
                 )
    );
  sup.onChildTerminated.connect
    (boost::bind ( s_child_terminated
                 , &sup
                 , _1
                 )
    );
  sup.onChildStarted.connect
    (boost::bind ( s_child_started
                 , &sup
                 , _1
                 )
    );

  sup.start ();

  rc = sup.add_child
    (gspc::rif::child_descriptor_t ( "ls"
                                   , argv
                                   , env
                                   , gspc::rif::child_descriptor_t::RESTART_ALWAYS
                                   , gspc::rif::child_descriptor_t::SHUTDOWN_KILL
                                   , 0
                                   )
    );

  BOOST_REQUIRE_EQUAL (rc, 0);

  sem.P ();
}

BOOST_AUTO_TEST_CASE (test_supervise_temporary)
{
  gspc::rif::argv_t argv;
  gspc::rif::env_t env;
  int rc;
  char buf [4096];
  fhg::thread::semaphore sem (0);

  memset (buf, 0, sizeof(buf));

  argv.push_back ("/bin/sleep");
  argv.push_back ("1");

  gspc::rif::manager_t mgr;
  mgr.start ();

  gspc::rif::supervisor_t sup (mgr, 2, 30);
  sup.onChildTerminated.connect
    (boost::bind ( s_child_terminated
                 , &sup
                 , _1
                 )
    );
  sup.onChildTerminated.connect
    (boost::bind ( s_wakeup
                 , &sem
                 )
    );
  sup.onChildStarted.connect
    (boost::bind ( s_child_started
                 , &sup
                 , _1
                 )
    );

  sup.start ();

  rc = sup.add_child
    (gspc::rif::child_descriptor_t ( "sleep"
                                   , argv
                                   , env
                                   , gspc::rif::child_descriptor_t::RESTART_NEVER
                                   , gspc::rif::child_descriptor_t::SHUTDOWN_KILL
                                   , 0
                                   )
    );

  BOOST_REQUIRE_EQUAL (rc, 0);

  sem.P ();
}

BOOST_AUTO_TEST_CASE (test_supervise_terminate_start)
{
  gspc::rif::argv_t argv;
  gspc::rif::env_t env;
  int rc;
  char buf [4096];
  bool done;
  fhg::thread::semaphore sem (0);

  memset (buf, 0, sizeof(buf));

  argv.push_back ("/bin/cat");

  gspc::rif::manager_t mgr;
  mgr.start ();

  gspc::rif::supervisor_t sup (mgr, 3, 10);
  sup.onChildFailed.connect
    (boost::bind ( s_child_failed
                 , &sup
                 , &done
                 , _1
                 )
    );
  sup.onChildFailed.connect
    (boost::bind ( s_wakeup
                 , &sem
                 )
    );
  sup.onChildTerminated.connect
    (boost::bind ( s_child_terminated
                 , &sup
                 , _1
                 )
    );
  sup.onChildTerminated.connect
    (boost::bind ( s_wakeup
                 , &sem
                 )
    );
  sup.onChildStarted.connect
    (boost::bind ( s_child_started
                 , &sup
                 , _1
                 )
    );
  sup.onChildStarted.connect
    (boost::bind ( s_wakeup
                 , &sem
                 )
    );

  sup.start ();

  rc = sup.add_child
    (gspc::rif::child_descriptor_t ( "cat"
                                   , argv
                                   , env
                                   , gspc::rif::child_descriptor_t::RESTART_ONLY_IF_FAILED
                                   , gspc::rif::child_descriptor_t::SHUTDOWN_INFINITY
                                   , 0
                                   )
    );

  BOOST_REQUIRE_EQUAL (rc, 0);
  {
    gspc::rif::supervisor_t::child_info_t info =
      sup.get_child_info ("cat");
    BOOST_REQUIRE (info.proc > 0);
  }

  sem.P ();

  std::cerr << "terminating cat" << std::endl;

  rc = sup.terminate_child ("cat");
  BOOST_REQUIRE_EQUAL (rc, 0);
  {
    gspc::rif::supervisor_t::child_info_t info =
      sup.get_child_info ("cat");
    BOOST_REQUIRE (info.proc <= 0);
  }

  sem.P ();

  std::cerr << "restarting cat" << std::endl;

  rc = sup.restart_child ("cat");

  sem.P ();

  BOOST_REQUIRE_EQUAL (rc, 0);
  {
    gspc::rif::supervisor_t::child_info_t info =
      sup.get_child_info ("cat");
    BOOST_REQUIRE (info.proc > 0);
  }

  sup.stop ();
}

BOOST_AUTO_TEST_SUITE_END()
