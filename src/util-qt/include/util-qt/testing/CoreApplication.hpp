// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-qt/testing/detail/FakeCommandLineArguments.hpp>

#include <QtCore/QCoreApplication>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace testing
      {
        //! A QCoreApplication that can be used in unit test
        //! environment. Shall be inside a test body.
        class CoreApplication : private detail::FakeCommandLineArguments
                              , public QCoreApplication
        {
        public:
          CoreApplication();
        };
      }
    }
  }
}
