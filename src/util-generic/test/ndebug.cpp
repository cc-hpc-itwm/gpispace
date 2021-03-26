// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <util-generic/ndebug.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    BOOST_AUTO_TEST_CASE (macro_says_same_thing_as_manual_ifdef)
    {
      bool has_ndebug =
#ifdef NDEBUG
      true
#else
      false
#endif
        ;

      bool macro_says = IFDEF_NDEBUG (true) IFNDEF_NDEBUG (false);

      BOOST_REQUIRE_EQUAL (has_ndebug, macro_says);
    }
  }
}
