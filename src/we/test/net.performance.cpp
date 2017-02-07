#include <boost/test/unit_test.hpp>

#include <we/type/expression.hpp>
#include <we/type/net.hpp>
#include <we/type/transition.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/chrono.hpp>

#include <chrono>
#include <random>

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
          ( !net.fire_expressions_and_extract_activity_random
              (random_engine(), unexpected_workflow_response)
          );

        auto const duration
          ( std::chrono::duration_cast<std::chrono::microseconds>
              (Clock::now() - start)
          );

        BOOST_REQUIRE (net.get_token (p).empty());

#define CHECK(_limit, _quality)                                         \
        if (last_duration > _limit)                                     \
        {                                                               \
          BOOST_CHECK_LT (last_duration * _quality, duration * 10);    \
        }

        CHECK (std::chrono::microseconds (100), 85);
        CHECK (std::chrono::milliseconds (1), 93);
        CHECK (std::chrono::milliseconds (10), 96);
        CHECK (std::chrono::milliseconds (100), 98);

#undef CHECK

        last_duration = duration;
      }
    }
  }
}
