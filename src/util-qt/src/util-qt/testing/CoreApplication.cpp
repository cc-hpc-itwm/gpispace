// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-qt/testing/CoreApplication.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace testing
      {
        CoreApplication::CoreApplication()
          : QCoreApplication (argc, argv.data())
        {}
      }
    }
  }
}
