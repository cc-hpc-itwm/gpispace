// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-qt/testing/Application.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace testing
      {
        Application::Application()
          : QApplication (argc, argv.data())
        {}
      }
    }
  }
}
