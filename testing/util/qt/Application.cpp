#include <gspc/testing/util/qt/Application.hpp>




      namespace gspc::util::qt::testing
      {
        Application::Application()
          : QApplication (argc, argv.data())
        {}
      }
