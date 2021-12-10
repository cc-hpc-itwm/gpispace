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
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/test_case.hpp>

#include <boost/range/adaptors.hpp>

#include <list>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE (simple)
{
  {
    using join = fhg::util::join_reference<std::list<int>, std::string>;

    BOOST_REQUIRE_EQUAL (join ({}, ", ").string(), "");
    BOOST_REQUIRE_EQUAL (join ({0}, ", ").string(), "0");
    BOOST_REQUIRE_EQUAL (join ({0, 1}, ", ").string(), "0, 1");
    BOOST_REQUIRE_EQUAL (join ({0, 1, 2}, ", ").string(), "0, 1, 2");
    BOOST_REQUIRE_EQUAL (join ({}, "").string(), "");
    BOOST_REQUIRE_EQUAL (join ({0}, "").string(), "0");
    BOOST_REQUIRE_EQUAL (join ({0, 1}, "").string(), "01");
    BOOST_REQUIRE_EQUAL (join ({0, 1, 2}, "").string(), "012");
  }

  {
    using join = fhg::util::join_reference<std::list<int>, int>;

    BOOST_REQUIRE_EQUAL (join ({}, 8).string(), "");
    BOOST_REQUIRE_EQUAL (join ({0}, 8).string(), "0");
    BOOST_REQUIRE_EQUAL (join ({0, 1}, 8).string(), "081");
    BOOST_REQUIRE_EQUAL (join ({0, 1, 2}, 8).string(), "08182");
  }

  {
    using join = fhg::util::join_reference<std::list<std::string>, std::string>;

    BOOST_REQUIRE_EQUAL (join ({}, ".").string(), "");
    BOOST_REQUIRE_EQUAL (join ({{}}, ".").string(), "");
    BOOST_REQUIRE_EQUAL (join ({{},{}}, ".").string(), ".");
    BOOST_REQUIRE_EQUAL (join ({{},{},{}}, ".").string(), "..");
    BOOST_REQUIRE_EQUAL (join ({""}, ".").string(), "");
    BOOST_REQUIRE_EQUAL (join ({"0"}, ".").string(), "0");
    BOOST_REQUIRE_EQUAL (join ({"0", "1"}, ".").string(), "0.1");
    BOOST_REQUIRE_EQUAL (join ({"0", "1", "2"}, ".").string(), "0.1.2");
  }
}

BOOST_AUTO_TEST_CASE (join_iterator)
{
  using join = fhg::util::join_iterator<std::list<int>::const_iterator, std::string>;

  std::list<int> list;
  BOOST_REQUIRE_EQUAL (join (list.begin(), list.end(), "-").string(), "");
  list.push_back (0);
  BOOST_REQUIRE_EQUAL (join (list.begin(), list.end(), "-").string(), "0");
  list.push_back (1);
  BOOST_REQUIRE_EQUAL (join (list.begin(), list.end(), "-").string(), "0-1");
}

BOOST_AUTO_TEST_CASE (join_reference_has_reference_to_container)
{
  std::list<int> list;
  fhg::util::join_reference<std::list<int>, std::string> const join (list, "-");
  BOOST_REQUIRE_EQUAL (join.string(), "");
  list.emplace_back (0);
  BOOST_REQUIRE_EQUAL (join.string(), "0");
  list.emplace_back (1);
  BOOST_REQUIRE_EQUAL (join.string(), "0-1");
}

BOOST_AUTO_TEST_CASE (callback_is_called)
{
  using join = fhg::util::join_reference<std::list<int>, char>;

  unsigned long call_count (0);

  fhg::util::ostream::callback::print_function<int> const print
    { [&call_count] (std::ostream& os, int const& x) -> std::ostream&
      {
        ++call_count;
        return os << '(' << x << ')';
      }
    };

  BOOST_REQUIRE_EQUAL (join ({0}, ' ', print).string(), "(0)");
  BOOST_REQUIRE_EQUAL (join ({0, 1}, ' ', print).string(), "(0) (1)");
  BOOST_REQUIRE_EQUAL (call_count, 3UL);

  BOOST_REQUIRE_EQUAL (join ({0}, ' ', print, "<", ">").string(), "<(0)>");
  BOOST_REQUIRE_EQUAL (join ({0, 1}, ' ', print, "<", ">").string(), "<(0) (1)>");
  BOOST_REQUIRE_EQUAL (call_count, 6UL);
}

BOOST_AUTO_TEST_CASE (callback_to_member)
{
  struct user_defined
  {
    user_defined (int i)
      : _i (i + 1)
    {}
    int const& i() const
    {
      return _i;
    }
    std::string string() const
    {
      return std::to_string (i() * 3);
    }
  private:
    int const _i;
  };

  using join = fhg::util::join_reference<std::list<user_defined>, char>;

  BOOST_REQUIRE_EQUAL
    ( join ( {0}, ' '
           , fhg::util::ostream::callback::select<user_defined, int>
               (&user_defined::i)
           ).string()
    , "1"
    );
  BOOST_REQUIRE_EQUAL
    ( join ( {0, 1}, ' '
           , fhg::util::ostream::callback::select<user_defined, int>
               (&user_defined::i)
           ).string()
    , "1 2"
    );
  BOOST_REQUIRE_EQUAL
    ( join ( {0}, ' '
           , fhg::util::ostream::callback::select<user_defined, std::string>
               (&user_defined::string)
           ).string()
    , "3"
    );
  BOOST_REQUIRE_EQUAL
    ( join ( {0, 1}, ' '
           , fhg::util::ostream::callback::select<user_defined, std::string>
               (&user_defined::string)
           ).string()
    , "3 6"
    );
}

namespace
{
  template<typename Container, typename Separator>
    void verify (std::size_t n, Separator separator)
  {
    Container const container (fhg::util::testing::randoms<Container> (n));
    typename Container::const_iterator begin {container.begin()};
    std::ostringstream expected;

    if (begin != container.end())
    {
      expected << *begin++;

      while (begin != container.end())
      {
        expected << separator << *begin++;
      }
    }

    {
      fhg::util::join_reference<Container, Separator> const join
        (container, separator);

      BOOST_REQUIRE_EQUAL (join.string(), expected.str());
      BOOST_REQUIRE_EQUAL (join.string(), expected.str());
    }

    {
      fhg::util::join_reference<Container, Separator> const join
        (container, separator);

      BOOST_REQUIRE_EQUAL (join.string(), expected.str());
      BOOST_REQUIRE_EQUAL (join.string(), expected.str());
    }

    {
      fhg::util::join_iterator<typename Container::const_iterator, Separator>
        const join (container.begin(), container.end(), separator);

      BOOST_REQUIRE_EQUAL (join.string(), expected.str());
      BOOST_REQUIRE_EQUAL (join.string(), expected.str());
    }
  }
}

FHG_UTIL_TESTING_TEMPLATED_CASE_T ( intense
                                  , Container
                                  , std::list<std::string>
                                  , std::list<int>
                                  , std::vector<std::string>
                                  , std::vector<int>
                                  )
{
  for (std::size_t n (0); n < 10; ++n) BOOST_TEST_CONTEXT ("n = " << n)
  {
    BOOST_TEST_CONTEXT ("separator = std::string")
      verify<Container> (n, fhg::util::testing::random<std::string>()());
    BOOST_TEST_CONTEXT ("separator = int")
      verify<Container> (n, fhg::util::testing::random<int>()());
  }
}

BOOST_AUTO_TEST_CASE (max_elements)
{
  using join = fhg::util::join_reference<std::list<int>, char>;
  using size_type = std::list<int>::const_iterator::difference_type;

  BOOST_REQUIRE_EQUAL
    ( join ( {0}, ' ', fhg::util::ostream::callback::id<int>()
           , "<", ">"
           , size_type (2)
           ).string()
    , "[1]: <0>"
    );
  BOOST_REQUIRE_EQUAL
    ( join ( {0, 1}, ' ', fhg::util::ostream::callback::id<int>()
           , "<", ">"
           , size_type (2)
           ).string()
    , "[2]: <0 1>"
    );
  BOOST_REQUIRE_EQUAL
    ( join ( {0, 1, 2}, ' ', fhg::util::ostream::callback::id<int>()
           , "<", ">"
           , size_type (2)
           ).string()
    , "[3]: <0 1...>"
    );
}

BOOST_AUTO_TEST_CASE (join_ranges)
{
  std::list<int> const list {0,1,2};

  BOOST_REQUIRE_EQUAL
    ( fhg::util::join
       ( list | ::boost::adaptors::transformed ([] (int i) { return 2 * i; })
       , " "
       ).string()
    , "0 2 4"
    );
}
