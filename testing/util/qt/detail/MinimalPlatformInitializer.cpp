#include <gspc/testing/util/qt/detail/MinimalPlatformInitializer.hpp>

#include <gspc/util/syscall.hpp>

#include <cstdlib>





        namespace gspc::util::qt::testing::detail
        {
          namespace
          {
            constexpr char const* const var = "QT_QPA_PLATFORM";
          }

          MinimalPlatformInitializer::MinimalPlatformInitializer()
            : _previous (std::getenv (var))
          {
            gspc::util::syscall::setenv (var, "minimal", 1);
          }
          MinimalPlatformInitializer::~MinimalPlatformInitializer()
          {
            if (_previous)
            {
              gspc::util::syscall::setenv (var, _previous, 1);
            }
          }
        }
