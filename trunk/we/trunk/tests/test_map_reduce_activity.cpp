#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <we/net.hpp>
#include <we/util/codec.hpp>
#include <we/type/transition.hpp>
#include <we/type/place.hpp>
#include <we/type/token.hpp>
#include <we/mgmt/type/activity.hpp>

#include "simple_example_generator.hpp"

using petri_net::connection_t;
using petri_net::PT;
using petri_net::PT_READ;
using petri_net::TP;

typedef place::type place_t;
typedef token::type token_t;
typedef unsigned int edge_t;
typedef we::type::transition_t<place_t, edge_t, token_t> transition_t;
typedef petri_net::net<place_t, transition_t, edge_t, token_t> pnet_t;
typedef we::mgmt::type::activity_t<transition_t, token_t> activity_t;
typedef activity_t::input_t input_t;

int main (int, char **)
{
  transition_t simple_trans ( we::tests::gen::simple<activity_t>::generate());

  std::cout << "simple transition:"
            << std::endl
            << simple_trans
            << std::endl;

  activity_t act ( simple_trans );

  act.input ().push_back
    ( input_t::value_type
      ( token_t ("", "long", 0L)
      , simple_trans.input_port_by_name ("input")
      )
    );

  std::cout << "act (original):"
            << std::endl
            << act
            << std::endl;
  {
    std::string act_encoded = we::util::text_codec::encode (act);
    std::cout << "act (serialized):"
              << std::endl
              << act_encoded
              << std::endl;

    activity_t act_d = we::util::text_codec::decode<activity_t> (act_encoded);
    std::cout << "act (deserialized):"
              << std::endl
              << act_d
              << std::endl;
  }

  if (act.execute ())
  {
    std::cout << "internal activity" << std::endl;
  }
  else
  {
    std::cout << "external activity" << std::endl;
  }

  std::cout << "has_enabled = " << act.has_enabled() << std::endl;
  while (act.has_enabled())
  {
    activity_t sub = act.extract ();

    std::cout << "***** sub-act (pre-execute):"
              << std::endl
              << sub
              << std::endl;

    const bool handle_internally = sub.execute ();

    if (! handle_internally)
    {
      std::cerr << "E: external activity not supported!" << std::endl;
    }

    std::cout << "***** sub-act (post-execute):"
              << std::endl
              << sub
              << std::endl;
    act.inject (sub);
  }

  we::mgmt::visitor::output_collector<activity_t> output_collector(act);
  boost::apply_visitor (output_collector, act.transition().data());

  std::cout << "act (finished) = "
            << std::endl
            << act
            << std::endl;

  if ( act.output().empty() )
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
