#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <we/type/net.hpp>
#include <we/util/codec.hpp>
#include <we/type/transition.hpp>
#include <we/type/place.hpp>
#include <we/type/token.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/mgmt/context.hpp>

#include <we/type/module_call.fwd.hpp>
#include <we/type/expression.hpp>
#include <we/type/net.fwd.hpp>

using petri_net::connection_t;
using petri_net::edge::PT;
using petri_net::edge::PT_READ;
using petri_net::edge::TP;

typedef we::type::transition_t transition_t;
typedef petri_net::net pnet_t;
typedef we::mgmt::type::activity_t activity_t;

struct exec_context : public we::mgmt::context
{
  virtual int handle_internally (activity_t&, net_t&)
  {
    return 0;
  }

  virtual int handle_internally (activity_t&, mod_t&)
  {
    return 0;
  }

  virtual int handle_internally (activity_t&, expr_t&)
  {
    return 0;
  }

  virtual int handle_externally (activity_t&, net_t&)
  {
    return 0;
  }

  virtual int handle_externally (activity_t&, mod_t&)
  {
    return 0;
  }

  virtual int handle_externally (activity_t&, expr_t&)
  {
    return 0;
  }
};


int main (int, char **)
{
  // ************************************ //
  pnet_t net;

  petri_net::place_id_type pid_vid (net.add_place (place::type ("vid","long")));

  signature::structured_t sig_store;
  sig_store["bid"] = "long";
  sig_store["seen"] = "bitset";

  petri_net::place_id_type pid_store (net.add_place (place::type("store", sig_store)));

  transition_t trans_inner
    ( "trans_inner"
    , we::type::expression_t
      ( "${store.seen} := bitset_insert (${store.seen}, ${vid});"
        "${store.bid}  := ${store.bid}                         ;"
        "${pair.bid}   := ${store.bid}                         ;"
        "${pair.vid}   := ${vid}                               "
      )
      , "!bitset_is_element (${store.seen}, ${vid})"
      , true
    );

  signature::structured_t sig_pair;

  sig_pair["bid"] = "long";
  sig_pair["vid"] = "long";

  petri_net::place_id_type pid_pair (net.add_place (place::type("pair", sig_pair)));

  trans_inner.add_port
    ("vid","long",we::type::PORT_IN);
  trans_inner.add_port
    ("store",sig_store,we::type::PORT_OUT);
  trans_inner.add_port
    ("store",sig_store,we::type::PORT_IN);
  trans_inner.add_port
    ("pair",sig_pair,we::type::PORT_OUT)
    ;

  trans_inner.add_connection
    (pid_vid,"vid");
  trans_inner.add_connection
    (pid_store,"store");
  trans_inner.add_connection
    ("pair",pid_pair);
  trans_inner.add_connection
    ("store",pid_store)
    ;

  petri_net::transition_id_type tid (net.add_transition (trans_inner));

  net.add_connection (connection_t (PT, tid, pid_store));
  net.add_connection (connection_t (TP, tid, pid_store));
  net.add_connection (connection_t (PT_READ, tid, pid_vid));
  net.add_connection (connection_t (TP, tid, pid_pair));

  token::put (net, pid_vid, literal::type(0L));
  token::put (net, pid_vid, literal::type(1L));

  {
    value::structured_t m;

    m["seen"] = bitsetofint::type(0);

    m["bid"] = 0L;
    token::put (net, pid_store, m);

    m["bid"] = 1L;
    token::put (net, pid_store, m);
  }
  // ************************************ //

  transition_t tnet ("tnet", net);
  tnet.add_port
    ("vid", "long", we::type::PORT_IN, pid_vid);
  tnet.add_port
    ("store", sig_store, we::type::PORT_IN, pid_store);
  tnet.add_port
    ("store", sig_store, we::type::PORT_OUT, pid_store);
  tnet.add_port
    ("pair", sig_pair, we::type::PORT_OUT, pid_pair)
    ;

  activity_t act ( tnet );

  std::cout << "act (original):"
            << std::endl
            << act
            << std::endl;
  {
    std::string act_encoded (act.to_string());
    std::cout << "act (serialized):"
              << std::endl
              << act_encoded
              << std::endl;

    activity_t act_d = we::util::codec::decode (act_encoded);
    std::cout << "act (deserialized):"
              << std::endl
              << act_d
              << std::endl;
  }

  std::cout << "can_fire = " << act.can_fire() << std::endl;
  while (act.can_fire())
  {
    activity_t sub = act.extract ();
    sub.inject_input ();

    std::cout << "***** sub-act (pre-execute):"
              << std::endl
              << sub
              << std::endl;

    exec_context ctxt;
    sub.execute (&ctxt);

    std::cout << "***** sub-act (post-execute):"
              << std::endl
              << sub
              << std::endl;
    act.inject (sub);
  }

  act.collect_output();

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
