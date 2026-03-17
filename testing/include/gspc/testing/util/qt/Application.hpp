#pragma once

#include <gspc/testing/util/qt/detail/FakeCommandLineArguments.hpp>
#include <gspc/testing/util/qt/detail/MinimalPlatformInitializer.hpp>

#include <QtWidgets/QApplication>




      namespace gspc::util::qt::testing
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
