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

#include <util-generic/finally.hpp>

#include <boost/preprocessor/cat.hpp>

#include <stack>

BOOST_AUTO_TEST_CASE (finally_is_called_on_scope_exit)
{
  std::stack<int> stack;

  {
    fhg::util::finally_t<std::function<void()>> const f0
      ([&stack] { stack.push (0); });

    {
      fhg::util::finally_t<std::function<void()>> const f1
        ([&stack] { stack.push (1); });

      BOOST_REQUIRE (stack.empty());
    }

    BOOST_REQUIRE_EQUAL (stack.size(), 1);
    BOOST_REQUIRE_EQUAL (stack.top(), 1);
  }

  BOOST_REQUIRE_EQUAL (stack.size(), 2);
  BOOST_REQUIRE_EQUAL (stack.top(), 0);
}

BOOST_AUTO_TEST_CASE (finally_constructed_from_named_function)
{
  std::stack<int> stack;

  auto&& push_value
    ( [&stack] (int i)
      {
        stack.push (i);
      }
    );

  auto&& push_zero
    ( [&push_value] ()
      {
        push_value (0);
      }
    );

  {
    fhg::util::finally_t<std::function<void()>> const f0
      (std::bind (push_zero));
    fhg::util::finally_t<std::function<void()>> const f1
      (std::bind (push_value, 1));
  }

  BOOST_REQUIRE_EQUAL (stack.size(), 2);
  BOOST_REQUIRE_EQUAL (stack.top(), 0);
  stack.pop();
  BOOST_REQUIRE_EQUAL (stack.top(), 1);
}

BOOST_AUTO_TEST_CASE (define_helper_macro)
{
#define TEST_FINALLY(fun)                              \
  fhg::util::finally_t<std::function<void()>> const    \
    BOOST_PP_CAT (_fhg_util_finally_, __LINE__) (fun)

  std::stack<int> stack;

  {
    TEST_FINALLY ([&stack] { stack.push (0); });
    TEST_FINALLY ([&stack] { stack.push (1); });
  }

  BOOST_REQUIRE_EQUAL (stack.size(), 2);
  BOOST_REQUIRE_EQUAL (stack.top(), 0);
  stack.pop();
  BOOST_REQUIRE_EQUAL (stack.top(), 1);

#undef TEST_FINALLY
}

BOOST_AUTO_TEST_CASE (use_predefined_macro)
{
  std::stack<int> stack;

  {
    FHG_UTIL_FINALLY ([&stack] { stack.push (0); });
    FHG_UTIL_FINALLY ([&stack] { stack.push (1); });
  }

  BOOST_REQUIRE_EQUAL (stack.size(), 2);
  BOOST_REQUIRE_EQUAL (stack.top(), 0);
  stack.pop();
  BOOST_REQUIRE_EQUAL (stack.top(), 1);
}

BOOST_AUTO_TEST_CASE (misuse_move_ctor)
{
  std::stack<int> stack;

  {
    fhg::util::finally_t<std::function<void()>> f0
      ([&stack] { stack.push (0); });

    fhg::util::finally_t<std::function<void()>> const f1
      ([&stack] { stack.push (1); });

    fhg::util::finally_t<std::function<void()>> const f2
      (std::move (f0));
  }

  BOOST_REQUIRE_EQUAL (stack.size(), 2);
  BOOST_REQUIRE_EQUAL (stack.top(), 1);
  stack.pop();
  BOOST_REQUIRE_EQUAL (stack.top(), 0);
}
