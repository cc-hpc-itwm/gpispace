#define BOOST_TEST_MODULE GpiSpaceTaskTest
#include <boost/test/unit_test.hpp>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/bind.hpp>

#include <fhglog/LogMacros.hpp>
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

static void task_executed (int *i)
{
  (*i)++;
}

static void task_erroneous ()
{
  throw std::runtime_error ("function failed");
}

BOOST_AUTO_TEST_CASE ( simple_task )
{
  using namespace gpi::pc::memory;
  int executed (0);
  task_t task ("simple_task", boost::bind (task_executed, &executed));

  BOOST_CHECK (task.USED_IN_TEST_ONLY_is_pending());

  task.execute ();
  task.wait ();

  BOOST_CHECK (task.has_finished());
  BOOST_CHECK_EQUAL (executed, 1);
}

BOOST_AUTO_TEST_CASE ( simple_task_cancel )
{
  using namespace gpi::pc::memory;
  int executed (0);
  task_t task ("simple_task", boost::bind (task_executed, &executed));

  BOOST_CHECK (task.USED_IN_TEST_ONLY_is_pending());

  task.cancel ();
  task.execute ();
  task.wait ();

  BOOST_CHECK (task.USED_IN_TEST_ONLY_was_cancelled());
  BOOST_CHECK_EQUAL (executed, 0);
}

BOOST_AUTO_TEST_CASE ( simple_task_failed )
{
  using namespace gpi::pc::memory;
  task_t task ("simple_task", task_erroneous);

  BOOST_CHECK (task.USED_IN_TEST_ONLY_is_pending());

  task.execute ();
  task.wait ();

  BOOST_CHECK (task.has_failed());
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
