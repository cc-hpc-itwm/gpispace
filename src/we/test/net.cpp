#include <boost/test/unit_test.hpp>

#include <we/type/expression.hpp>
#include <we/type/net.hpp>
#include <we/type/transition.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random_string.hpp>
#include <util-generic/testing/printer/chrono.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <chrono>
#include <random>
#include <sstream>

namespace
{
  void unexpected_workflow_response
    (pnet::type::value::value_type const&, pnet::type::value::value_type const&)
  {
    throw std::logic_error ("got unexpected workflow_response");
  }

  std::mt19937& random_engine()
  {
    static std::mt19937 _ {std::random_device{}()};

    return _;
  }

  we::type::property::type no_properties()
  {
    return {};
  }
}

BOOST_AUTO_TEST_CASE (transition_without_input_port_can_not_fire)
{
  we::type::net_type net;
  net.add_transition ( we::type::transition_t
                       ( fhg::util::testing::random_string()
                       , we::type::expression_t()
                       , boost::none
                       , no_properties()
                       , we::priority_type()
                       )
                     );

  BOOST_REQUIRE
    ( !net.fire_expressions_and_extract_activity_random
        (random_engine(), unexpected_workflow_response)
    );
}

BOOST_AUTO_TEST_CASE (deserialized_transition_without_input_port_can_not_fire)
{
  std::stringstream iostream;
  boost::archive::text_oarchive oar (iostream);

  {
    we::type::net_type net;
    net.add_transition ( we::type::transition_t
                         ( fhg::util::testing::random_string()
                         , we::type::expression_t()
                         , boost::none
                         , no_properties()
                         , we::priority_type()
                         )
                       );

    oar << net;
  }

  boost::archive::text_iarchive iar (iostream);

  we::type::net_type net;
  iar >> net;

  BOOST_REQUIRE
    ( !net.fire_expressions_and_extract_activity_random
        (random_engine(), unexpected_workflow_response)
    );
}

BOOST_AUTO_TEST_CASE (transition_that_depends_on_own_output_can_fire)
{
  pnet::type::signature::signature_type const signature
    (std::string ("control"));

  we::type::net_type net;

  auto&& add_place
    ([&]() -> we::place_id_type
     {
       static int place {0};

       return net.add_place
         (place::type ( std::to_string (++place)
                      , signature
                      , boost::none
                      , no_properties()
                      )
         );
     }
    );

  we::place_id_type const place_in (add_place());
  we::place_id_type const place_out (add_place());
  we::place_id_type const place_credit (add_place());

  net.put_value (place_in, we::type::literal::control());
  net.put_value (place_credit, we::type::literal::control());

  we::type::transition_t transition
    ( fhg::util::testing::random_identifier()
    , we::type::expression_t ("${out} := ${in}")
    , boost::none
    , no_properties()
    , we::priority_type()
    );

  auto&& add_port
    ([&] ( std::string const& name
         , we::type::PortDirection direction
         ) -> we::port_id_type
     {
       return transition.add_port
         (we::type::port_t (name, direction, signature, no_properties()));
     }
    );

  we::port_id_type const port_in (add_port ("in", we::type::PORT_IN));
  we::port_id_type const port_out (add_port ("out", we::type::PORT_OUT));
  we::port_id_type const port_credit_in (add_port ("c", we::type::PORT_IN));
  we::port_id_type const port_credit_out (add_port ("c", we::type::PORT_OUT));

  we::transition_id_type const transition_id (net.add_transition (transition));

  auto&& connect
    ([&] (we::edge::type edge, we::place_id_type place, we::port_id_type port)
     {
       net.add_connection (edge, transition_id, place, port, no_properties());
     }
    );

  connect (we::edge::PT, place_in, port_in);
  connect (we::edge::TP, place_out, port_out);
  connect (we::edge::PT, place_credit, port_credit_in);
  connect (we::edge::TP, place_credit, port_credit_out);

  BOOST_REQUIRE_EQUAL (net.get_token (place_in).size(), 1);
  BOOST_REQUIRE (net.get_token (place_out).empty());
  BOOST_REQUIRE_EQUAL (net.get_token (place_credit).size(), 1);

  BOOST_REQUIRE
    ( !net.fire_expressions_and_extract_activity_random
        (random_engine(), unexpected_workflow_response)
    );

  BOOST_REQUIRE (net.get_token (place_in).empty());
  BOOST_REQUIRE_EQUAL (net.get_token (place_out).size(), 1);
  BOOST_REQUIRE_EQUAL (net.get_token (place_credit).size(), 1);
}

namespace we
{
  namespace type
  {
    BOOST_AUTO_TEST_CASE (empty_net)
    {
      net_type net;

      BOOST_REQUIRE (net.places().empty());
      BOOST_REQUIRE (net.transitions().empty());
      BOOST_REQUIRE (net.transition_to_place().empty());
      BOOST_REQUIRE (net.place_to_transition_consume().empty());
      BOOST_REQUIRE (net.place_to_transition_read().empty());
      BOOST_REQUIRE (net.port_to_place().empty());
      BOOST_REQUIRE (net.port_to_response().empty());
      BOOST_REQUIRE (net.place_to_port().empty());
      BOOST_REQUIRE
        ( !net.fire_expressions_and_extract_activity_random
            (random_engine(), unexpected_workflow_response)
        );
    }

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
