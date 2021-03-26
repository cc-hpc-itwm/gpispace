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

#include <boost/test/unit_test.hpp>

#include <util-generic/join.hpp>
#include <util-generic/print_container.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/test_case.hpp>

#include <cstddef>
#include <list>
#include <string>
#include <vector>

FHG_UTIL_TESTING_TEMPLATED_CASE_T ( print_container
                                  , Container
                                  , std::list<int>
                                  , std::list<std::string>
                                  , std::vector<int>
                                  , std::vector<std::string>
                                  )
{
  for (std::size_t n (0); n < 10; ++n) BOOST_TEST_CONTEXT ("n = " << n)
  {
    Container const container {fhg::util::testing::randoms<Container> (n)};
    std::string const open {fhg::util::testing::random<std::string>()()};
    std::string const separator {fhg::util::testing::random<std::string>()()};
    std::string const close {fhg::util::testing::random<std::string>()()};

    using join = fhg::util::join_reference<Container, std::string>;

    BOOST_REQUIRE_EQUAL
      ( open + join (container, separator).string() + close
      , fhg::util::print_container (open, separator, close, container).string()
      );
  }
}
