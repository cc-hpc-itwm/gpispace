// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-qt/testing/detail/MinimalPlatformInitializer.hpp>

#include <util-generic/syscall.hpp>

#include <cstdlib>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace testing
      {
        namespace detail
        {
          namespace
          {
            constexpr char const* const var = "QT_QPA_PLATFORM";
          }

          MinimalPlatformInitializer::MinimalPlatformInitializer()
            : _previous (std::getenv (var))
          {
            fhg::util::syscall::setenv (var, "minimal", 1);
          }
          MinimalPlatformInitializer::~MinimalPlatformInitializer()
          {
            if (_previous)
            {
              fhg::util::syscall::setenv (var, _previous, 1);
            }
          }
        }
      }
    }
  }
}
