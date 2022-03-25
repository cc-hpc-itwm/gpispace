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

#include <util-generic/nest_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    BOOST_AUTO_TEST_CASE (does_nothing_if_not_throwing)
    {
      BOOST_REQUIRE
        (nest_exceptions<std::runtime_error> ([] { return true; }, "unused"));
    }

    BOOST_AUTO_TEST_CASE (wraps_exception_with_specified_type_and_args)
    {
      auto const inner (testing::random<std::string>{}());
      auto const outer (testing::random<std::string>{}());

      testing::require_exception
        ( [&]
          {
            nest_exceptions<std::logic_error>
              ( [&] { throw std::runtime_error (inner); }
              , outer
              );
          }
        , testing::make_nested
            (std::logic_error (outer), std::runtime_error (inner))
        );
    }
  }
}
