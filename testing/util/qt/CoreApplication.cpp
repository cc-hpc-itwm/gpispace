#include <gspc/testing/util/qt/CoreApplication.hpp>




      namespace gspc::util::qt::testing
      {
        CoreApplication::CoreApplication()
          : QCoreApplication (argc, argv.data())
        {}
      }
