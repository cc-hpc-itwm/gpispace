// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <util-generic/join.hpp>
#include <util-generic/print_container.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/test_case.hpp>

#include <cstddef>
#include <iostream>
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

namespace fhg
{
  namespace util
  {
    namespace
    {
      struct NeitherCopyNorMoveable
      {
        // https://bugs.llvm.org/show_bug.cgi?id=25084 prevents from writing
        // NeitherCopyNorMoveable() = default;
        NeitherCopyNorMoveable() {}
        NeitherCopyNorMoveable (NeitherCopyNorMoveable const&) = delete;
        NeitherCopyNorMoveable (NeitherCopyNorMoveable&&) = delete;
        NeitherCopyNorMoveable& operator= (NeitherCopyNorMoveable const&) = delete;
        NeitherCopyNorMoveable& operator= (NeitherCopyNorMoveable&&) = delete;
        // https://bugs.llvm.org/show_bug.cgi?id=25084 prevents from writing
        // ~NeitherCopyNorMoveable() = default;
        ~NeitherCopyNorMoveable() {}
      };
      std::ostream& operator<< (std::ostream& os, NeitherCopyNorMoveable const&)
      {
        return os << '.';
      }
    }

    BOOST_AUTO_TEST_CASE (noncopyormoveables_can_be_printed)
    {
      std::vector<NeitherCopyNorMoveable> xs
        (fhg::util::testing::random<std::size_t>{} (1000));

      std::string const expected (xs.size(), '.');
      auto const output (print_container ({}, {}, {}, xs).string());

      BOOST_REQUIRE_EQUAL (output, expected);
    }
  }
}
