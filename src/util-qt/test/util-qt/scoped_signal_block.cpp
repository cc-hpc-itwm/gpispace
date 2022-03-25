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

#include <util-qt/scoped_signal_block.hpp>

#include <QtCore/QObject>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      BOOST_AUTO_TEST_CASE (blocks_signals_from_emitting_and_resets)
      {
        QObject object;

        BOOST_REQUIRE_EQUAL (object.signalsBlocked(), false);

        {
          scoped_signal_block const block {&object};

          BOOST_REQUIRE_EQUAL (object.signalsBlocked(), true);
        }

        BOOST_REQUIRE_EQUAL (object.signalsBlocked(), false);
      }

      BOOST_AUTO_TEST_CASE (does_not_unblock_if_blocked_before)
      {
        QObject object;

        object.blockSignals (true);

        {
          scoped_signal_block const block {&object};
        }

        BOOST_REQUIRE_EQUAL (object.signalsBlocked(), true);
      }

      BOOST_AUTO_TEST_CASE
        (restores_blocking_state_even_if_changed_during_lifetime)
      {
        QObject object;

        {
          scoped_signal_block const block {&object};

          {
            scoped_signal_block const block_innner {&object};

            object.blockSignals (false);
          }

          BOOST_REQUIRE_EQUAL (object.signalsBlocked(), true);
        }
      }
    }
  }
}
