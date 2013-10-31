#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <we/type/net.hpp>
#include <we/type/transition.hpp>
#include <we/type/place.hpp>
#include <we/type/value.hpp>
#include <we/type/port.hpp>
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

  petri_net::place_id_type pid_vid (net.add_place (place::type ("vid",std::string("long"))));

  pnet::type::signature::structure_type sig_store_fields;

  sig_store_fields.push_back (std::make_pair ( std::string ("bid")
                                             , std::string ("long")
                                             )
                             );
  sig_store_fields.push_back (std::make_pair ( std::string ("seen")
                                             , std::string ("bitset")
                                             )
                             );

  pnet::type::signature::structured_type sig_store
    (std::make_pair (std::string ("store"), sig_store_fields));

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

  pnet::type::signature::structure_type sig_pair_fields;

  sig_pair_fields.push_back (std::make_pair ( std::string ("bid")
                                            , std::string ("long")
                                            )
                            );
  sig_pair_fields.push_back (std::make_pair ( std::string ("vid")
                                            , std::string ("long")
                                            )
                            );

  pnet::type::signature::structured_type sig_pair
    (std::make_pair (std::string ("pair"), sig_pair_fields));

  petri_net::place_id_type pid_pair (net.add_place (place::type("pair", sig_pair)));

  trans_inner.add_port
    (we::type::port_t ("vid",we::type::PORT_IN,std::string("long")));
  trans_inner.add_port
    (we::type::port_t ("store",we::type::PORT_OUT,sig_store));
  trans_inner.add_port
    (we::type::port_t ("store",we::type::PORT_IN,sig_store));
  trans_inner.add_port
    (we::type::port_t ("pair",we::type::PORT_OUT,sig_pair));

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

  net.put_value (pid_vid, 0L);
  net.put_value (pid_vid, 1L);

  {
    pnet::type::value::structured_type m;

    m.push_back (std::make_pair (std::string ("bid"), 0L));
    m.push_back (std::make_pair (std::string ("seen"), bitsetofint::type(0)));

    net.put_value (pid_store, m);
  }

  {
    pnet::type::value::structured_type m;

    m.push_back (std::make_pair (std::string ("bid"), 1L));
    m.push_back (std::make_pair (std::string ("seen"), bitsetofint::type(0)));

    net.put_value (pid_store, m);
  }

  // ************************************ //

  transition_t tnet ("tnet", net);
  tnet.add_port
    (we::type::port_t ("vid", we::type::PORT_IN, std::string("long"), pid_vid));
  tnet.add_port
    (we::type::port_t ("store", we::type::PORT_IN, sig_store, pid_store));
  tnet.add_port
    (we::type::port_t ("store", we::type::PORT_OUT, sig_store, pid_store));
  tnet.add_port
    (we::type::port_t ("pair", we::type::PORT_OUT, sig_pair, pid_pair));

  activity_t act ( tnet );

  {
    std::string act_encoded (act.to_string());
    std::cout << "act (serialized):"
              << std::endl
              << act_encoded
              << std::endl;
  }

  std::cout << "can_fire = " << act.can_fire() << std::endl;
  while (act.can_fire())
  {
    activity_t sub = act.extract ();
    sub.inject_input ();

    exec_context ctxt;
    sub.execute (&ctxt);

    act.inject (sub);
  }

  act.collect_output();

  if ( act.output().empty() )
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
