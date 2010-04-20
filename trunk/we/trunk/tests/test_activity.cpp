#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <we/net.hpp>
#include <we/util/codec.hpp>
#include <we/type/transition.hpp>
#include <we/type/place.hpp>
#include <we/type/token.hpp>
#include <we/mgmt/type/activity.hpp>

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

int main (int, char **)
{
  // ************************************ //
  pnet_t net("the inner net");

  petri_net::pid_t pid_vid (net.add_place (place_t ("vid","long")));

  signature::structured_t sig_store;

  sig_store["bid"] = "long";
  sig_store["seen"] = "bitset";

  petri_net::pid_t pid_store (net.add_place (place::type("store", sig_store)));

  transition_t trans_inner 
    ( "trans_inner"
    , transition_t::expr_type
      ( "${store.seen} := bitset_insert (${store.seen}, ${vid}); \
         ${store.bid}  := ${store.bid}                         ; \
         ${pair.bid}   := ${store.bid}                         ; \
         ${pair.vid}   := ${vid}                                 "
      )
    , "!bitset_is_element (${store.seen}, ${vid})"
    , true
    );

  signature::structured_t sig_pair;

  sig_pair["bid"] = "long";
  sig_pair["vid"] = "long";

  petri_net::pid_t pid_pair (net.add_place (place::type("pair", sig_pair)));

  trans_inner.add_ports ()
    ("vid","long",we::type::PORT_IN)
    ("store",sig_store,we::type::PORT_IN_OUT)
    ("pair",sig_pair,we::type::PORT_OUT)
    ;

  trans_inner.add_connections ()
    (pid_vid,"vid")
    (pid_store,"store")
    ("pair",pid_pair)
    ("store",pid_store)
    ;

  petri_net::tid_t tid (net.add_transition (trans_inner));

  edge_t e (0);

  net.add_edge (e++, connection_t (PT, tid, pid_store));
  net.add_edge (e++, connection_t (TP, tid, pid_store));
  net.add_edge (e++, connection_t (PT_READ, tid, pid_vid));
  net.add_edge (e++, connection_t (TP, tid, pid_pair));
  
  token::put (net, pid_vid, literal::type(0L));

  {
    value::structured_t m;

    m["bid"] = 0L;
    m["seen"] = bitsetofint::type(0);

    token::put (net, pid_store, m);
  }
  // ************************************ //

  transition_t tnet ("tnet", transition_t::net_type (net));
  tnet.add_ports()
    ("vid", "long", we::type::PORT_IN, pid_vid)
    ("store", sig_store, we::type::PORT_IN_OUT, pid_store)
    ("pair", sig_pair, we::type::PORT_OUT, pid_pair)
  ;

  activity_t act ( 0, tnet );

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

  std::cout << "has_enabled = " << act.has_enabled() << std::endl;
  if (act.has_enabled())
  {
    activity_t sub = act.extract (1);
    std::cout << "sub-act:"
              << std::endl
              << sub
              << std::endl;
    sub.execute ("");
  }

  return EXIT_SUCCESS;
}
