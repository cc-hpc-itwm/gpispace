// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <util-generic/ostream/prefix_per_line.hpp>
#include <util-generic/testing/random.hpp>

#include <sstream>
#include <string>
#include <vector>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      namespace
      {
        std::string const any_single_line
          {testing::random<std::string>::except ("\n")};
      }

      BOOST_AUTO_TEST_CASE (each_line_gets_the_prefix)
      {
        using fhg::util::testing::random;

        std::ostringstream oss;
        std::ostringstream expected;

        auto const prefix (random<std::string>{}());

        prefix_per_line _ (prefix, oss);

        auto n (random<std::size_t>{} (1000));

        while (n --> 0)
        {
          auto const line (random<std::string>{} (any_single_line));

          expected << prefix << line << '\n';
          _ << line << '\n';
        }

        BOOST_REQUIRE_EQUAL (expected.str(), oss.str());
      }
    }
  }
}
