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

#include <we/type/activity.hpp>
#include <we/type/expression.hpp>
#include <we/type/net.hpp>
#include <we/type/transition.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/chrono.hpp>

#include <algorithm>
#include <chrono>
#include <random>
#include <vector>

#include <we/test/net.common.hpp>

namespace we
{
  namespace type
  {
    BOOST_AUTO_TEST_CASE (scalability_in_countdown)
    {
      pnet::type::signature::signature_type const signature
        (std::string ("unsigned long"));

      transition_t dec ( "dec"
                       , expression_t ("${out} := ${in} - 1UL")
                       , expression_t ("${in} :gt: 0UL")
                       , no_properties()
                       , priority_type()
                       , boost::optional<we::type::eureka_id_type>{}
                       , std::list<we::type::preference_t>{}
                       );
      port_id_type const in
        ( dec.add_port
            (port_t (std::string ("in"), PORT_IN, signature, no_properties()))
        );
      port_id_type const out
        ( dec.add_port
            (port_t (std::string ("out"), PORT_OUT, signature, no_properties()))
        );

      transition_t eat ( "eat"
                       , expression_t()
                       , expression_t ("${x} :le: 0UL")
                       , no_properties()
                       , priority_type()
                       , boost::optional<we::type::eureka_id_type>{}
                       , std::list<we::type::preference_t>{}
                       );
      port_id_type const x
        ( eat.add_port
            (port_t (std::string ("x"), PORT_IN, signature, no_properties()))
        );

      net_type net;

      transition_id_type const d (net.add_transition (dec));
      transition_id_type const e (net.add_transition (eat));

      place_id_type const p
        ( net.add_place
            (place::type ("value", signature, false, no_properties()))
        );

      net.add_connection (edge::PT, d, p, in, no_properties());
      net.add_connection (edge::TP, d, p, out, no_properties());
      net.add_connection (edge::PT, e, p, x, no_properties());

      unsigned long number_of_tokens (0);

      using Clock = std::chrono::steady_clock;
      using Duration = std::chrono::microseconds;

      Duration last_duration {0};

      for ( unsigned long n (1)
          ; n <= 10000000
          ; number_of_tokens += 1 + n, n *= 10
          )
      {
        net.put_value (p, pnet::type::value::value_type (n));

        BOOST_REQUIRE_EQUAL (net.get_token (p).size(), 1);
        BOOST_REQUIRE_EQUAL ( net.get_token (p).begin()->first
                            , token_id_type (number_of_tokens)
                            );
        BOOST_REQUIRE_EQUAL ( net.get_token (p).begin()->second
                            , pnet::type::value::value_type (n)
                            );

        auto const start (Clock::now());

        BOOST_REQUIRE
          ( !net.fire_expressions_and_extract_activity_random_TESTING_ONLY
              (random_engine(), unexpected_workflow_response, unexpected_eureka)
          );

        auto const duration
          ( std::chrono::duration_cast<std::chrono::microseconds>
              (Clock::now() - start)
          );

        BOOST_REQUIRE (net.get_token (p).empty());

#define CHECK(_limit, _quality)                                         \
        do                                                              \
        if (last_duration > _limit)                                     \
        {                                                               \
          BOOST_CHECK_LT (last_duration * _quality, duration * 10);     \
        } while (false)

        CHECK (std::chrono::microseconds (100), 85);
        CHECK (std::chrono::milliseconds (1), 93);
        CHECK (std::chrono::milliseconds (10), 96);
        CHECK (std::chrono::milliseconds (100), 98);

#undef CHECK

        last_duration = duration;
      }
    }

    BOOST_AUTO_TEST_CASE (partial_cross_product_uses_referenced_places_only)
    {
      using Clock = std::chrono::steady_clock;
      using Duration = decltype (Clock::now() - Clock::now());

      std::vector<Duration> durations;

      for ( std::size_t number_of_tokens {1}
          ; number_of_tokens <= 10000000
          ; number_of_tokens *= 10
          )
      {
        //    (x::bool) -> [ t        ]
        // (y::control) -> [ if: ${x} ]

        net_type net;

        auto add_place
          ( [&] ( std::string name
                , pnet::type::signature::signature_type signature
                )
            {
              return net.add_place
                ({name, signature, boost::none, no_properties()});
            }
          );

        auto const pid_x (add_place ("x", "bool"));
        auto const pid_y (add_place ("y", "control"));

        transition_t transition ( "t"
                                , type::expression_t{}
                                , expression_t {"${x}"}
                                , no_properties()
                                , we::priority_type{}
                                , boost::optional<we::type::eureka_id_type>{}
                                , std::list<we::type::preference_t>{}
                                );

        auto add_in_port
          ( [&] ( std::string name
                , pnet::type::signature::signature_type signature
                )
            {
              return transition.add_port
                ({name, PORT_IN, signature, no_properties()});
            }
          );

        auto const port_x (add_in_port ("x", "bool"));
        auto const port_y (add_in_port ("y", "control"));

        auto const tid (net.add_transition (transition));

        net.add_connection (edge::PT, tid, pid_x, port_x, no_properties());
        net.add_connection (edge::PT, tid, pid_y, port_y, no_properties());

        // t is not enabled because there is no token on x, the
        // condition is not evaluated
        for (decltype (number_of_tokens) _ {0}; _ < number_of_tokens; ++_)
        {
          net.put_value (pid_y, type::literal::control());
        }

        // to put a token on x implies the evaluation of the condition
        auto const start (Clock::now());
        net.put_value (pid_x, false);
        auto const end (Clock::now());

        durations.emplace_back (end - start);
      }

      auto minmax (std::minmax_element (durations.cbegin(), durations.cend()));

      // the factor 100 is very small compared to the growth in the
      // number of tokens in y, to not differ by more than a factor of
      // 100 is a good indicator that the condition evaluation does
      // not depend on the number of tokens on place y
      BOOST_REQUIRE_LT (((*minmax.second) / (*minmax.first)), 100);
    }
  }
}
