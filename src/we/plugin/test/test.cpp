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

#include <we/plugin/Plugins.hpp>
#include <we/type/value/boost/test/printer.hpp>
#include <we/type/value/show.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/filesystem/path.hpp>

#include <stdexcept>
#include <string>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      namespace
      {
        namespace tf = ::boost::unit_test::framework;

        struct FPlugins
        {
          FPlugins()
            : value (fhg::util::testing::random<long>{}())
            , tag (fhg::util::testing::random<std::string>{}())
            , expected_value (value)
            , expected_tag (tag)
          {
            if (tf::master_test_suite().argc != 5)
            {
              BOOST_FAIL ("missing paths to plugins");
            }

            context.bind_and_discard_ref ({"A"}, value);
            context.bind_and_discard_ref ({"B"}, tag);
          }

          std::string A = tf::master_test_suite().argv[1];
          std::string B = tf::master_test_suite().argv[2];
          std::string C = tf::master_test_suite().argv[3];
          std::string D = tf::master_test_suite().argv[4];

          Plugins plugins;
          expr::eval::context context;
          pnet::type::value::value_type value;
          std::string tag;
          decltype (value) expected_value;
          decltype (tag) expected_tag;
          unsigned call = 0;
          unsigned call1 = 0;
          unsigned call2 = 0;
        };
      }

#define REQUIRE_CALLBACK_AT(var, f, args...)    \
        do                                      \
        {                                       \
          auto const expected (var + 1U);       \
          plugins.f (args, context);            \
          BOOST_REQUIRE_EQUAL (var, expected);  \
        } while (0)

#define REQUIRE_CALLBACK(f, args...)            \
        REQUIRE_CALLBACK_AT (call, f, args)

#define REQUIRE_CALLBACK_VALUE_AT(var, v, f, args...)   \
        expected_value = v;                             \
        REQUIRE_CALLBACK_AT (var, f, args)

#define REQUIRE_CALLBACK_VALUE(v, f, args...)   \
        expected_value = v;                     \
        REQUIRE_CALLBACK (f, args)

#define UNEXPECTED()                                                    \
        [&] (std::string _tag, pnet::type::value::value_type _value)    \
        {                                                               \
          BOOST_FAIL                                                    \
            ( "UNEXPECTED "                                             \
            << "(" << _tag                                              \
            << ", " << pnet::type::value::show (_value)                 \
            << ")"                                                      \
            );                                                          \
        }

#define EXPECT_AT(var,t,v)                                              \
        [&] (std::string _tag, pnet::type::value::value_type _value)    \
        {                                                               \
          BOOST_REQUIRE_EQUAL (_tag, t);                                \
          BOOST_REQUIRE_EQUAL (_value, v);                              \
          ++var;                                                        \
        }
#define EXPECT(t,v) EXPECT_AT (call,t,v)

      BOOST_FIXTURE_TEST_CASE (call_after_add_works, FPlugins)
      {
        auto const pid (plugins.create (A, context, EXPECT ("A", value)));

        REQUIRE_CALLBACK (before_eval, pid);
        REQUIRE_CALLBACK (after_eval, pid);
      }

      BOOST_FIXTURE_TEST_CASE (call_twice_after_add_works, FPlugins)
      {
        auto const pid (plugins.create (A, context, EXPECT ("A", value)));

        REQUIRE_CALLBACK (before_eval, pid);
        REQUIRE_CALLBACK (after_eval, pid);
        REQUIRE_CALLBACK (before_eval, pid);
        REQUIRE_CALLBACK (after_eval, pid);
      }

      BOOST_FIXTURE_TEST_CASE (call_after_remove_throws, FPlugins)
      {
        auto const pid (plugins.create (A, context, UNEXPECTED()));
        plugins.destroy (pid);

        fhg::util::testing::require_exception
          ( [&]
            {
              plugins.before_eval (pid, context);
            }
          , fhg::util::testing::make_nested
            ( std::runtime_error
              (str ( ::boost::format ("Plugins::before_eval (%1%, %2%)")
                   % to_string (pid)
                   % context
                   )
              )
            , std::invalid_argument ("Plugins: Unknown " + to_string (pid))
            )
          );
        fhg::util::testing::require_exception
          ( [&]
            {
              plugins.after_eval (pid, context);
            }
          , fhg::util::testing::make_nested
            ( std::runtime_error
              (str ( ::boost::format ("Plugins::after_eval (%1%, %2%)")
                   % to_string (pid)
                   % context
                   )
              )
            , std::invalid_argument ("Plugins: Unknown " + to_string (pid))
            )
          );
      }

      BOOST_FIXTURE_TEST_CASE (erase_unknown_throws, FPlugins)
      {
        auto const pid (plugins.create (A, context, UNEXPECTED()));
        plugins.destroy (pid);

        fhg::util::testing::require_exception
          ( [&]
            {
              plugins.destroy (pid);
            }
          , std::invalid_argument ("Plugins: Unknown " + to_string (pid))
          );
      }

      BOOST_FIXTURE_TEST_CASE (plugin_can_have_a_state, FPlugins)
      {
        auto const pid
          (plugins.create (B, context, EXPECT (tag, expected_value)));

        REQUIRE_CALLBACK_VALUE (0U, before_eval, pid);
        REQUIRE_CALLBACK_VALUE (1U, before_eval, pid);
        REQUIRE_CALLBACK_VALUE (0U, after_eval, pid);
        REQUIRE_CALLBACK_VALUE (1U, after_eval, pid);
      }

      BOOST_FIXTURE_TEST_CASE
        ( multiple_plugins_can_exists_at_the_same_time
        , FPlugins
        )
      {
        auto const pid1
          (plugins.create (A, context, EXPECT_AT (call1, "A", value)));
        auto const pid2
          (plugins.create (A, context, EXPECT_AT (call2, "A", value)));

        REQUIRE_CALLBACK_AT (call1, before_eval, pid1);
        REQUIRE_CALLBACK_AT (call2, before_eval, pid2);
      }

      BOOST_FIXTURE_TEST_CASE
        ( call_works_after_other_plugin_of_the_same_kind_was_erased
        , FPlugins
        )
      {
        auto const pid1 (plugins.create (A, context, UNEXPECTED()));
        auto const pid2(plugins.create (A, context, EXPECT ("A", value)));
        plugins.destroy (pid1);

        REQUIRE_CALLBACK (before_eval, pid2);
      }

      BOOST_FIXTURE_TEST_CASE
        ( multiple_stateful_plugins_can_exists_at_the_same_time
        , FPlugins
        )
      {
        auto const tag1 (tag + "-1");
        auto const tag2 (tag + "-2");

        context.bind_and_discard_ref ({"B"}, tag1);

        auto const pid1
          ( plugins.create
              (B, context, EXPECT_AT (call1, tag1, expected_value))
          );

        context.bind_and_discard_ref ({"B"}, tag2);

        auto const pid2
          ( plugins.create
              (B, context, EXPECT_AT (call2, tag2, expected_value))
          );

        REQUIRE_CALLBACK_VALUE_AT (call1, 0U, before_eval, pid1);
        REQUIRE_CALLBACK_VALUE_AT (call1, 1U, before_eval, pid1);

        REQUIRE_CALLBACK_VALUE_AT (call2, 0U, before_eval, pid2);
        REQUIRE_CALLBACK_VALUE_AT (call2, 1U, before_eval, pid2);

        REQUIRE_CALLBACK_VALUE_AT (call1, 2U, before_eval, pid1);

        REQUIRE_CALLBACK_VALUE_AT (call1, 0U, after_eval, pid1);
        REQUIRE_CALLBACK_VALUE_AT (call1, 1U, after_eval, pid1);

        REQUIRE_CALLBACK_VALUE_AT (call2, 0U, after_eval, pid2);
        REQUIRE_CALLBACK_VALUE_AT (call2, 1U, after_eval, pid2);

        REQUIRE_CALLBACK_VALUE_AT (call1, 2U, after_eval, pid1);
      }

      BOOST_FIXTURE_TEST_CASE
        ( call_works_after_other_stateful_plugin_of_the_same_kind_was_erased
        , FPlugins
        )
      {
        auto const pid1
          (plugins.create (B, context, EXPECT (tag, expected_value)));

        REQUIRE_CALLBACK_VALUE (0U, before_eval, pid1);

        auto const pid2 (plugins.create (B, context, UNEXPECTED()));
        plugins.destroy (pid2);

        REQUIRE_CALLBACK_VALUE (1U, before_eval, pid1);
      }

      BOOST_FIXTURE_TEST_CASE
        ( multiple_plugins_of_different_kind_can_exists_at_the_same_time
        , FPlugins
        )
      {
        auto const pidA
          (plugins.create (A, context, EXPECT_AT (call1, "A", value)));
        auto const pidB
          (plugins.create (B, context, EXPECT_AT (call2, tag, expected_value)));

        REQUIRE_CALLBACK_AT (call1, before_eval, pidA);
        REQUIRE_CALLBACK_VALUE_AT (call2, 0U, before_eval, pidB);
        REQUIRE_CALLBACK_VALUE_AT (call2, 1U, before_eval, pidB);
        REQUIRE_CALLBACK_AT (call1, before_eval, pidA);
        REQUIRE_CALLBACK_VALUE_AT (call2, 2U, before_eval, pidB);
      }

      BOOST_FIXTURE_TEST_CASE (plugin_ctor_can_call_back, FPlugins)
      {
        plugins.create (C, context, EXPECT (tag, value));
        BOOST_REQUIRE_EQUAL (call, 1U);
      }

      BOOST_FIXTURE_TEST_CASE (throw_from_call_propagates, FPlugins)
      {
        auto const pid (plugins.create (C, context, EXPECT (tag, value)));

        fhg::util::testing::require_exception
          ( [&]
            {
              plugins.before_eval (pid, context);
            }
          , fhg::util::testing::make_nested
            ( std::runtime_error
              (str ( ::boost::format ("Plugins::before_eval (%1%, %2%)")
                   % to_string (pid)
                   % context
                   )
              )
            , std::runtime_error ("C::before_eval()")
            )
          );
        fhg::util::testing::require_exception
          ( [&]
            {
              plugins.after_eval (pid, context);
            }
          , fhg::util::testing::make_nested
            ( std::runtime_error
              (str ( ::boost::format ("Plugins::after_eval (%1%, %2%)")
                   % to_string (pid)
                   % context
                   )
              )
            , std::runtime_error ("C::after_eval()")
            )
          );
      }

      BOOST_FIXTURE_TEST_CASE (throw_from_ctor_propagates, FPlugins)
      {
        fhg::util::testing::require_exception
          ( [&]
            {
              plugins.create (D, context, UNEXPECTED());
            }
          , fhg::util::testing::make_nested
            ( std::runtime_error
              (str ( ::boost::format ("Plugins::create (%1%, %2%)")
                   % ::boost::filesystem::path {D}
                   % context
                   )
              )
            , std::runtime_error ("Exception in gspc_we_plugin_create: D::D()")
            )
          );
      }
    }
  }
}
