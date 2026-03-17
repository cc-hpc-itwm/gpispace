// Copyright (C) 2011-2013,2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include <stdexcept>



    namespace gspc::util::qt
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
