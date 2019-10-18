#include <boost/test/unit_test.hpp>

#include <we/type/expression.hpp>
#include <we/type/net.hpp>
#include <we/type/transition.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random/string.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <algorithm>
#include <chrono>
#include <random>
#include <sstream>
#include <vector>

#include <we/test/net.common.hpp>

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
