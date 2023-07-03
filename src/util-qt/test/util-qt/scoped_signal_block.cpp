// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
