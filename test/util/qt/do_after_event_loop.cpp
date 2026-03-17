#include <gspc/util/qt/do_after_event_loop.hpp>

#include <gspc/testing/util/qt/CoreApplication.hpp>

#include <boost/test/unit_test.hpp>



    namespace gspc::util::qt
    {
      BOOST_AUTO_TEST_CASE (function_is_not_executed_without_event_loop)
      {
        testing::CoreApplication app;

        int was_called (0);
        do_after_event_loop ([&] { ++was_called; });

        BOOST_REQUIRE_EQUAL (was_called, 0);
      }

      BOOST_AUTO_TEST_CASE (function_is_called_when_processing_events)
      {
        testing::CoreApplication app;

        int was_called (0);
        do_after_event_loop ([&] { ++was_called; });

        app.processEvents();

        BOOST_REQUIRE_EQUAL (was_called, 1);
      }

      BOOST_AUTO_TEST_CASE (function_is_called_only_once)
      {
        testing::CoreApplication app;

        int was_called (0);
        do_after_event_loop ([&] { ++was_called; });

        app.processEvents();
        app.processEvents();
        app.processEvents();
        app.processEvents();

        BOOST_REQUIRE_EQUAL (was_called, 1);
      }
    }
