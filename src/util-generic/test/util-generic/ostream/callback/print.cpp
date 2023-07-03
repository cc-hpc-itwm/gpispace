// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/ostream/callback/print.hpp>
#include <util-generic/testing/printer/vector.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      namespace callback
      {
        namespace
        {
          struct : std::ostream {} void_ostream;
        }

        BOOST_AUTO_TEST_CASE (prints_given_value_with_given_function)
        {
          auto value (testing::random<int>{}());
          auto const orig_value (value);

          std::vector<int> called_with;
          print<int> const p
            { [&] (std::ostream& os, int const& x) -> std::ostream&
              {
                called_with.emplace_back (x);
                return os;
              }
            , value
            };

          //! \note ensure it is captured by copy
          ++value;

          BOOST_REQUIRE_EQUAL (called_with, std::vector<int> {});
          p (void_ostream);
          BOOST_REQUIRE_EQUAL (called_with, std::vector<int> {orig_value});
          p (void_ostream);
          BOOST_REQUIRE_EQUAL
            (called_with, (std::vector<int> {orig_value, orig_value}));
        }
      }
    }
  }
}
