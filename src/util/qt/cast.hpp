// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      template<typename T> T throwing_qobject_cast (QObject* from)
      {
        T x (qobject_cast<T> (from));

        if (!x)
        {
          throw std::runtime_error ("throwing_qobject_cast failed");
        }

        return x;
      }
    }
  }
}
