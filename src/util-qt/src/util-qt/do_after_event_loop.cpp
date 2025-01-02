// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-qt/do_after_event_loop.hpp>

#include <QtCore/QTimer>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      void do_after_event_loop (std::function<void()> fun)
      {
        auto* timer (new QTimer);
        timer->setSingleShot (true);
        QObject::connect ( timer
                         , &QTimer::timeout
                         , [fun, timer]
                           {
                             fun();
                             timer->deleteLater();
                           }
                         );
        timer->start();
      }
    }
  }
}
