#define BOOST_TEST_MODULE GpiSpaceTaskTest
#include <boost/test/unit_test.hpp>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/bind.hpp>

#include <fhglog/minimal.hpp>
#include <boost/make_shared.hpp>
#include <gpi-space/pc/memory/task.hpp>
#include <gpi-space/pc/memory/task_queue.hpp>

struct SetupLogging
{
  SetupLogging ()
  {
    FHGLOG_SETUP();
  }
};

BOOST_GLOBAL_FIXTURE( SetupLogging );

struct F
{
  F()
  {}

  ~F()
  {}
};

static void task_executed (int *i)
{
  (*i)++;
}

static void task_erroneous ()
{
  throw std::runtime_error ("function failed");
}

BOOST_FIXTURE_TEST_SUITE( suite, F )

BOOST_AUTO_TEST_CASE ( simple_task )
{
  using namespace gpi::pc::memory;
  int executed (0);
  task_t task ("simple_task", boost::bind (task_executed, &executed));

  BOOST_CHECK_EQUAL (task.get_state(), task_state::pending);

  task.execute ();
  task.wait ();

  BOOST_CHECK_EQUAL (task.get_state(), task_state::finished);
  BOOST_CHECK_EQUAL (executed, 1);
}

BOOST_AUTO_TEST_CASE ( simple_task_cancel )
{
  using namespace gpi::pc::memory;
  int executed (0);
  task_t task ("simple_task", boost::bind (task_executed, &executed));

  BOOST_CHECK_EQUAL (task.get_state(), task_state::pending);

  task.cancel ();
  task.execute ();
  task.wait ();

  BOOST_CHECK_EQUAL (task.get_state(), task_state::cancelled);
  BOOST_CHECK_EQUAL (executed, 0);
}

BOOST_AUTO_TEST_CASE ( simple_task_failed )
{
  using namespace gpi::pc::memory;
  task_t task ("simple_task", task_erroneous);

  BOOST_CHECK_EQUAL (task.get_state(), task_state::pending);

  task.execute ();
  task.wait ();

  BOOST_CHECK_EQUAL (task.get_state(), task_state::failed);
}

BOOST_AUTO_TEST_CASE ( task_queue )
{
  using namespace gpi::pc::memory;
  int executed (0);
  task_queue_t q;

  const int count (10);
  for (int i = 0; i < count; ++i)
  {
    q.push
      (boost::make_shared<task_t>( "simple_task"
                                 , boost::bind(task_executed, &executed)
                                 )
      );
  }

  BOOST_CHECK_EQUAL (q.empty(), false);
  BOOST_CHECK_EQUAL (q.size(),  size_t(count));

  while (!q.empty())
  {
    task_queue_t::task_ptr t (q.pop());
    t->execute();
  }

  BOOST_CHECK_EQUAL (q.empty(), true);
  BOOST_CHECK_EQUAL (q.size(),  0u);
  BOOST_CHECK_EQUAL (executed, count);
}

BOOST_AUTO_TEST_SUITE_END()
