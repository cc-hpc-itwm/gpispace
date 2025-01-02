// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

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
          //! "Command line" arguments to pass to Qt (which requires
          //! non-const references).
          struct FakeCommandLineArguments
          {
            FakeCommandLineArguments();
            ~FakeCommandLineArguments() = default;
            FakeCommandLineArguments (FakeCommandLineArguments const&) = delete;
            FakeCommandLineArguments (FakeCommandLineArguments&&) = delete;
            FakeCommandLineArguments& operator= (FakeCommandLineArguments const&) = delete;
            FakeCommandLineArguments& operator= (FakeCommandLineArguments&&) = delete;

            std::vector<char> argv_data;
            std::vector<char*> argv;
            int argc;
          };
        }
      }
    }
  }
}
