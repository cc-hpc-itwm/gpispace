// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

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
          //! Initialize the platform plugin global state to minimal.
          struct MinimalPlatformInitializer
          {
            MinimalPlatformInitializer();
            ~MinimalPlatformInitializer();

            MinimalPlatformInitializer (MinimalPlatformInitializer const&) = delete;
            MinimalPlatformInitializer (MinimalPlatformInitializer&&) = delete;
            MinimalPlatformInitializer& operator= (MinimalPlatformInitializer const&) = delete;
            MinimalPlatformInitializer& operator= (MinimalPlatformInitializer&&) = delete;

          private:
            char const* _previous;
          };
        }
      }
    }
  }
}
