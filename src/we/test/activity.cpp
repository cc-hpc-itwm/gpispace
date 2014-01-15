#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <we/type/net.hpp>
#include <we/type/transition.hpp>
#include <we/type/place.hpp>
#include <we/type/value.hpp>
#include <we/type/port.hpp>
#include <we/type/activity.hpp>
#include <we/context.hpp>

#include <we/type/module_call.fwd.hpp>
#include <we/type/expression.hpp>
#include <we/type/net.fwd.hpp>

using we::edge::PT;
using we::edge::PT_READ;
using we::edge::TP;

typedef we::type::transition_t transition_t;
typedef we::net pnet_t;
typedef we::type::activity_t activity_t;

struct exec_context : public we::context
{
  virtual void handle_internally (activity_t&, net_t const&)
  {
  }

  virtual void handle_internally (activity_t&, mod_t const&)
  {
  }

  virtual void handle_internally (activity_t&, expr_t const&)
  {
  }

  virtual void handle_externally (activity_t&, net_t const&)
  {
  }

  virtual void handle_externally (activity_t&, mod_t const&)
  {
  }

  virtual void handle_externally (activity_t&, expr_t const&)
  {
  }
};


int main (int, char **)
{
  // ************************************ //
  pnet_t net;

  we::place_id_type pid_vid (net.add_place (place::type ("vid",std::string("long"))));

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

  we::place_id_type pid_store (net.add_place (place::type("store", sig_store)));

  transition_t trans_inner
    ( "trans_inner"
    , we::type::expression_t
      ( "${store.seen} := bitset_insert (${store.seen}, ${vid});"
        "${store.bid}  := ${store.bid}                         ;"
        "${pair.bid}   := ${store.bid}                         ;"
        "${pair.vid}   := ${vid}                               "
      )
    , condition::type ("!bitset_is_element (${store.seen}, ${vid})")
    , true
    , we::type::property::type()
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

  we::place_id_type pid_pair (net.add_place (place::type("pair", sig_pair)));

  we::port_id_type const port_id_vid
    ( trans_inner.add_port
      (we::type::port_t ("vid",we::type::PORT_IN,std::string("long")))
    );
  we::port_id_type const port_id_store_out
    ( trans_inner.add_port
      (we::type::port_t ("store",we::type::PORT_OUT,sig_store))
    );
  we::port_id_type const port_id_store_in
    ( trans_inner.add_port
      (we::type::port_t ("store",we::type::PORT_IN,sig_store))
    );
  we::port_id_type const& port_id_pair
    ( trans_inner.add_port
      (we::type::port_t ("pair",we::type::PORT_OUT,sig_pair))
    );

  we::transition_id_type tid (net.add_transition (trans_inner));

  {
    we::type::property::type empty;

    net.add_connection (PT, tid, pid_store, port_id_store_in, empty);
    net.add_connection (TP, tid, pid_store, port_id_store_out, empty);
    net.add_connection (PT_READ, tid, pid_vid, port_id_vid, empty);
    net.add_connection (TP, tid, pid_pair, port_id_pair, empty);
  }

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

  transition_t tnet ("tnet", net
                    , condition::type ("true")
                    , true, we::type::property::type()
                    );
  tnet.add_port
    (we::type::port_t ("vid", we::type::PORT_IN, std::string("long"), pid_vid));
  tnet.add_port
    (we::type::port_t ("store", we::type::PORT_IN, sig_store, pid_store));
  tnet.add_port
    (we::type::port_t ("store", we::type::PORT_OUT, sig_store, pid_store));
  tnet.add_port
    (we::type::port_t ("pair", we::type::PORT_OUT, sig_pair, pid_pair));

  activity_t act ( tnet, boost::none );

  {
    std::string act_encoded (act.to_string());
    std::cout << "act (serialized):"
              << std::endl
              << act_encoded
              << std::endl;
  }

  boost::mt19937 engine;

  if (act.transition().net())
  {
    while ( boost::optional<we::type::activity_t> sub
          = boost::get<we::net&> (act.transition().data())
          . fire_expressions_and_extract_activity_random (engine)
          )
    {
      exec_context ctxt;
      sub->execute (&ctxt);
      act.inject (*sub);
    }
  }

  if ( act.output().empty() )
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
