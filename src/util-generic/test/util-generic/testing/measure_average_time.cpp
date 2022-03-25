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

#include <util-generic/testing/measure_average_time.hpp>
#include <util-generic/testing/printer/chrono.hpp>

#include <boost/test/unit_test.hpp>

#include <thread>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      BOOST_AUTO_TEST_CASE (measures_time_somewhat_exact)
      {
        auto time
          ( measure_average_time<std::chrono::milliseconds>
              ( []
                {
                  std::this_thread::sleep_for (std::chrono::milliseconds (10));
                }
              , 1
              )
          );

        BOOST_REQUIRE_LE (time, std::chrono::milliseconds (11));
        BOOST_REQUIRE_GE (time, std::chrono::milliseconds (9));
      }

      BOOST_AUTO_TEST_CASE (gives_average)
      {
        std::size_t invocation (0);
        auto time
          ( measure_average_time<std::chrono::milliseconds>
              ( [&]
                {
                  switch (invocation)
                  {
                  case 0:
                    std::this_thread::sleep_for
                      (std::chrono::milliseconds (5));
                    break;
                  case 1:
                  case 2:
                    break;
                  case 3:
                    std::this_thread::sleep_for
                      (std::chrono::milliseconds (15));
                    break;
                  }
                  ++invocation;
                }
              , 4
              )
          );

        BOOST_REQUIRE_LE (time, std::chrono::milliseconds (6));
        BOOST_REQUIRE_GE (time, std::chrono::milliseconds (4));
      }
    }
  }
}
