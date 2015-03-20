#define BOOST_TEST_MODULE GpiSpaceTaskTest
#include <boost/test/unit_test.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <gpi-space/pc/memory/task.hpp>

BOOST_AUTO_TEST_CASE ( simple_task )
{
  using namespace gpi::pc::memory;
  int executed (0);
  task_t task ("simple_task", [&executed] { ++executed; });

  BOOST_CHECK (task.USED_IN_TEST_ONLY_is_pending());

  task.execute ();
  task.wait ();

  BOOST_CHECK (task.USED_IN_TEST_ONLY_has_finished());
  BOOST_CHECK_EQUAL (executed, 1);
}

BOOST_AUTO_TEST_CASE ( simple_task_failed )
{
  using namespace gpi::pc::memory;
  task_t task ("simple_task", [] { throw std::runtime_error ("function failed"); });

  BOOST_CHECK (task.USED_IN_TEST_ONLY_is_pending());

  task.execute ();
  task.wait ();

  BOOST_CHECK (task.has_failed());
}
