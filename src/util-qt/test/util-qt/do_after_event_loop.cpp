// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <util-qt/do_after_event_loop.hpp>

#include <util-qt/testing/CoreApplication.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
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
  }
}
