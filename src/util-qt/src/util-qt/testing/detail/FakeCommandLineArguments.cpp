// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-qt/testing/detail/FakeCommandLineArguments.hpp>

#include <util-generic/warning.hpp>

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
          FakeCommandLineArguments::FakeCommandLineArguments()
          {
            argv_data.emplace_back ('t');
            argv_data.emplace_back ('e');
            argv_data.emplace_back ('s');
            argv_data.emplace_back ('t');
            argv_data.emplace_back ('\0');

            argv.emplace_back (argv_data.data());

            argc = fhg::util::suppress_warning::sign_conversion<int>
              ( fhg::util::suppress_warning::shorten_64_to_32_with_check<unsigned int>
                  ( argv.size(), "As per insertion above, size == 1ul.")
              , "As per insertion and cast above, size == 1ul."
              );
          }
        }
      }
    }
  }
}
