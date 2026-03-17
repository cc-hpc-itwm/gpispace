#pragma once

#include <gspc/testing/util/qt/detail/FakeCommandLineArguments.hpp>

#include <QtCore/QCoreApplication>




      namespace gspc::util::qt::testing
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
