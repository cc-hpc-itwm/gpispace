// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
