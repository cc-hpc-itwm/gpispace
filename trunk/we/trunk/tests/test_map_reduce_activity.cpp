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

template <typename Context>
static void execute_loop ( activity_t & act, Context & ctxt )
{
  act.execute (ctxt);

  while (act.has_enabled())
  {
    activity_t sub = act.extract ();

    std::cout << "***** sub-act (pre-execute):"
              << std::endl
              << sub
              << std::endl;

    execute_loop (sub, ctxt);

    std::cout << "***** sub-act (post-execute):"
              << std::endl
              << sub
              << std::endl;
    act.inject (sub);
  }
}

struct exec_context
{
  typedef transition_t::net_type net_t;
  typedef transition_t::mod_type mod_t;
  typedef transition_t::expr_type expr_t;

  void handle_internally ( activity_t & , const net_t & )
  {
    // submit to self
  }

  void handle_internally ( activity_t & , const mod_t & )
  {
    //
  }

  void handle_internally ( activity_t & , const expr_t & )
  {
    // nothing to do
  }

  void handle_externally ( activity_t & , const net_t & )
  {
    // submit to sdpa
  }

  void handle_externally ( activity_t & , const mod_t & )
  {
    // submit to sdpa
  }

  void handle_externally ( activity_t & , const expr_t & )
  {
    // throw
  }
};

int main (int, char **)
{
  transition_t simple_trans ( we::tests::gen::simple<activity_t>::generate());

  std::cout << "simple transition:"
            << std::endl
            << simple_trans
            << std::endl;

  activity_t act ( simple_trans );

  for (std::size_t i (0); i < 1; ++i)
  {
    act.input ().push_back
      ( input_t::value_type
        ( token_t ("", "long", long(i))
        , simple_trans.input_port_by_name ("input")
        )
      );
  }

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

  struct exec_context ctxt;
  execute_loop (act, ctxt);

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
