// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-qt/testing/detail/FakeCommandLineArguments.hpp>
#include <util-qt/testing/detail/MinimalPlatformInitializer.hpp>

#include <QtWidgets/QApplication>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace testing
      {
        //! A QApplication that can be used in a `$DISPLAY`-less unit
        //! test environment. Shall be inside a test body.
        class Application : private detail::FakeCommandLineArguments
                          , private detail::MinimalPlatformInitializer
                          , public QApplication
        {
        public:
          Application();
        };
      }
    }
  }
}
