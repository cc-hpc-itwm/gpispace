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

#include <util-generic/cxx14/get_by_type.hpp>
#include <util-generic/cxx17/void_t.hpp>
#include <util-generic/testing/require_compiletime.hpp>

namespace fhg
{
  namespace util
  {
    namespace cxx14
    {
      //! \note The standard only specifies that the program will be
      //! ill-formed for these cases, but does not require that to be
      //! SFINAE-able, which it isn't e.g. in libstdcxx, making these
      //! tests fail, even if the same code would not compile when
      //! directly written. Our own implementation does provide that
      //! extra feature so test that we have it when using our
      //! implementation, at least.
#if ! HAS_STD_GET_BY_TYPE

      namespace
      {
        template<typename, typename, typename = void>
          struct get_is_callable : std::false_type {};

        template<typename T, typename Tuple>
          struct get_is_callable
            < T
            , Tuple
            , cxx17::void_t<decltype (get<T> (std::declval<Tuple>()))>
            > : std::true_type {};
      }

      BOOST_AUTO_TEST_CASE (should_not_be_callable_with_empty_tuple)
      {
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE
          (!get_is_callable<int, std::tuple<>>{});
      }

      BOOST_AUTO_TEST_CASE
        (should_not_be_callable_if_requested_element_is_missing)
      {
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE
          (!get_is_callable<int, std::tuple<char>>{});
      }

      BOOST_AUTO_TEST_CASE
        (should_not_be_callable_if_requested_type_is_duplicate)
      {
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE
          (!get_is_callable<int, std::tuple<int, int>>{});
      }

#endif

      BOOST_AUTO_TEST_CASE (should_be_callable_with_type_being_single_element)
      {
        BOOST_REQUIRE_EQUAL (get<int> (std::tuple<int> {1}), 1);
      }

      BOOST_AUTO_TEST_CASE (should_be_callable_with_type_existing_once)
      {
        BOOST_REQUIRE_EQUAL (get<int> (std::tuple<int, char> {13, 12}), 13);
        BOOST_REQUIRE_EQUAL (get<int> (std::tuple<char, int> {13, 12}), 12);
        BOOST_REQUIRE_EQUAL
          (get<int> (std::tuple<char, int, char> {1, 2, 3}), 2);
      }

      BOOST_AUTO_TEST_CASE (should_work_for_reference_types)
      {
        int x (1);
        BOOST_REQUIRE_EQUAL (get<int&> (std::tuple<int&> {x}), 1);
      }
    }
  }
}

BOOST_AUTO_TEST_CASE (by_ref)
{
  std::tuple<int, float> t (0, 0.0f);

  fhg::util::cxx14::get<int> (t) = 1;

  BOOST_REQUIRE_EQUAL (std::get<0> (t), 1);
  BOOST_REQUIRE_EQUAL (std::get<1> (t), 0.0f);
}

namespace
{
  struct copy_counter
  {
    copy_counter()
      : copies (0)
      , moves (0)
    {}
    copy_counter (copy_counter const& other)
      : copies (other.copies + 1)
      , moves (other.moves)
    {}
    copy_counter (copy_counter&& other)
      : copies (other.copies)
      , moves (other.moves + 1)
    {}
    copy_counter& operator= (copy_counter const& other) = delete;
    copy_counter& operator= (copy_counter&&) = delete;

    std::size_t copies;
    std::size_t moves;
  };
}

BOOST_AUTO_TEST_CASE (no_copy_or_move)
{
  {
    auto t (std::make_tuple (copy_counter{}));
    BOOST_REQUIRE_EQUAL (std::get<0> (t).copies, 0);
    BOOST_REQUIRE_EQUAL (std::get<0> (t).moves, 1);

    auto const& got (fhg::util::cxx14::get<copy_counter> (t));

    BOOST_REQUIRE_EQUAL (std::get<0> (t).copies, 0);
    BOOST_REQUIRE_EQUAL (std::get<0> (t).moves, 1);
    BOOST_REQUIRE_EQUAL (got.copies, 0);
    BOOST_REQUIRE_EQUAL (got.moves, 1);
  }

  {
    auto const t (std::make_tuple (copy_counter{}));
    BOOST_REQUIRE_EQUAL (std::get<0> (t).copies, 0);
    BOOST_REQUIRE_EQUAL (std::get<0> (t).moves, 1);

    auto const& got (fhg::util::cxx14::get<copy_counter> (t));

    BOOST_REQUIRE_EQUAL (std::get<0> (t).copies, 0);
    BOOST_REQUIRE_EQUAL (std::get<0> (t).moves, 1);
    BOOST_REQUIRE_EQUAL (got.copies, 0);
    BOOST_REQUIRE_EQUAL (got.moves, 1);
  }
}
