#include <gspc/util/in_this_scope.hpp>

#include <gspc/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <utility>


  namespace gspc::util
  {
    BOOST_AUTO_TEST_CASE (in_this_scope_sets_and_restores_value)
    {
      gspc::testing::unique_random<int> unique_random_int;

      auto const value (unique_random_int());
      auto const other_value (unique_random_int());

      int variable = value;

      {
        FHG_UTIL_IN_THIS_SCOPE (variable) = other_value;

        BOOST_REQUIRE_EQUAL (variable, other_value);
      }

      BOOST_REQUIRE_EQUAL (variable, value);
    }

    BOOST_AUTO_TEST_CASE
      (in_this_scope_works_correct_when_scope_is_left_via_an_exception)
    {
      gspc::testing::unique_random<int> unique_random_int;

      auto const value (unique_random_int());
      auto const other_value (unique_random_int());

      int variable = value;

      try
      {
        FHG_UTIL_IN_THIS_SCOPE (variable) = other_value;

        BOOST_REQUIRE_EQUAL (variable, other_value);

        throw unique_random_int();
      }
      catch (...)
      {
        std::ignore = std::current_exception();
      }

      BOOST_REQUIRE_EQUAL (variable, value);
    }

    BOOST_AUTO_TEST_CASE (in_this_scope_can_be_done_multiple_times)
    {
      gspc::testing::unique_random<int> unique_random_int;

      auto const value (unique_random_int());
      auto const other_value (unique_random_int());
      auto const and_another_value (unique_random_int());

      int variable = value;

      {
        FHG_UTIL_IN_THIS_SCOPE (variable) = other_value;

        BOOST_REQUIRE_EQUAL (variable, other_value);

        FHG_UTIL_IN_THIS_SCOPE (variable) = and_another_value;

        BOOST_REQUIRE_EQUAL (variable, and_another_value);
      }

      BOOST_REQUIRE_EQUAL (variable, value);
    }

    BOOST_AUTO_TEST_CASE (in_this_scope_can_be_nested)
    {
      gspc::testing::unique_random<int> unique_random_int;

      auto const value (unique_random_int());
      auto const other_value (unique_random_int());
      auto const and_another_value (unique_random_int());

      int variable = value;

      {
        FHG_UTIL_IN_THIS_SCOPE (variable) = other_value;

        {
          FHG_UTIL_IN_THIS_SCOPE (variable) = and_another_value;

          BOOST_REQUIRE_EQUAL (variable, and_another_value);
        }

        BOOST_REQUIRE_EQUAL (variable, other_value);
      }

      BOOST_REQUIRE_EQUAL (variable, value);
    }

    namespace
    {
      struct A
      {
      };
      struct B
      {
        operator A() const
        {
          return {};
        }
      };
      struct C
      {
        C() = default;
        C (A) {}
      };
    }

    BOOST_AUTO_TEST_CASE (in_this_scope_forwards_arguments_to_constructor)
    {
      C c;

      FHG_UTIL_IN_THIS_SCOPE (c) = B{};
    }

    namespace
    {
      struct S
      {
        S() : value (0) {}
        S (int x, int y) : value (x + y) {}

        int value;
      };
    }

    BOOST_AUTO_TEST_CASE
      (in_this_scope_has_alternative_syntax_to_delay_construction)
    {
      gspc::testing::random<int> random_int;

      auto const x (random_int());
      auto const y (random_int());

      S s;

      {
        FHG_UTIL_IN_THIS_SCOPE (s) (x, y);

        BOOST_REQUIRE_EQUAL (s.value, x + y);
      }

      BOOST_REQUIRE_EQUAL (s.value, 0);

      {
        FHG_UTIL_IN_THIS_SCOPE (s) = S (x, y);

        BOOST_REQUIRE_EQUAL (s.value, x + y);
      }

      BOOST_REQUIRE_EQUAL (s.value, 0);
    }
  }
