// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
