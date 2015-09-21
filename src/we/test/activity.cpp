#define BOOST_TEST_MODULE we_activity
#include <boost/test/unit_test.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <we/context.hpp>
#include <we/type/activity.hpp>
#include <we/type/net.hpp>
#include <we/type/value/poke.hpp>

namespace
{
  struct exec_context : public we::context
  {
    virtual void handle_internally (we::type::activity_t&, net_t const&) override {}
    virtual void handle_externally (we::type::activity_t&, net_t const&) override {}
    virtual void handle_externally (we::type::activity_t&, mod_t const&) override {}
  };
}

BOOST_AUTO_TEST_CASE (create_and_execute_cross_product)
{
  we::type::net_type net;

  we::place_id_type const pid_vid
    (net.add_place (place::type ("vid", std::string ("unsigned long"), boost::none)));

  pnet::type::signature::structure_type sig_store_fields;

  sig_store_fields.push_back
    (std::make_pair (std::string ("bid"), std::string ("unsigned long")));
  sig_store_fields.push_back
    (std::make_pair (std::string ("seen"), std::string ("bitset")));

  pnet::type::signature::structured_type const sig_store
    (std::make_pair (std::string ("store"), sig_store_fields));

  we::place_id_type const pid_store
    (net.add_place (place::type ("store", sig_store, boost::none)));

  we::type::transition_t trans_inner
    ( "trans_inner"
    , we::type::expression_t
      ( "${store.seen} := bitset_insert (${store.seen}, ${vid});"
        "${store.bid}  := ${store.bid}                         ;"
        "${pair.bid}   := ${store.bid}                         ;"
        "${pair.vid}   := ${vid}                               "
      )
    , we::type::expression_t ("!bitset_is_element (${store.seen}, ${vid})")
    , true
    , we::type::property::type()
    , we::priority_type()
    );

  pnet::type::signature::structure_type sig_pair_fields;

  sig_pair_fields.push_back
    (std::make_pair (std::string ("bid"), std::string ("unsigned long")));
  sig_pair_fields.push_back
    (std::make_pair (std::string ("vid"), std::string ("unsigned long")));

  pnet::type::signature::structured_type const sig_pair
    (std::make_pair (std::string ("pair"), sig_pair_fields));

  we::place_id_type const pid_pair
    (net.add_place (place::type("pair", sig_pair, boost::none)));

  we::port_id_type const port_id_vid
    ( trans_inner.add_port
        (we::type::port_t ("vid", we::type::PORT_IN, std::string("unsigned long")))
    );
  we::port_id_type const port_id_store_out
    ( trans_inner.add_port
        (we::type::port_t ("store", we::type::PORT_OUT, sig_store))
    );
  we::port_id_type const port_id_store_in
    ( trans_inner.add_port
        (we::type::port_t ("store", we::type::PORT_IN, sig_store))
    );
  we::port_id_type const& port_id_pair
    ( trans_inner.add_port
        (we::type::port_t ("pair", we::type::PORT_OUT, sig_pair))
    );

  we::transition_id_type const tid (net.add_transition (trans_inner));

  {
    we::type::property::type empty;

    net.add_connection (we::edge::PT, tid, pid_store, port_id_store_in, empty);
    net.add_connection (we::edge::TP, tid, pid_store, port_id_store_out, empty);
    net.add_connection (we::edge::PT_READ, tid, pid_vid, port_id_vid, empty);
    net.add_connection (we::edge::TP, tid, pid_pair, port_id_pair, empty);
  }

  net.put_value (pid_vid, 0UL);
  net.put_value (pid_vid, 1UL);

  {
    pnet::type::value::structured_type m;

    m.push_back (std::make_pair (std::string ("bid"), 0UL));
    m.push_back (std::make_pair (std::string ("seen"), bitsetofint::type(0)));

    net.put_value (pid_store, m);
  }

  {
    pnet::type::value::structured_type m;

    m.push_back (std::make_pair (std::string ("bid"), 1UL));
    m.push_back (std::make_pair (std::string ("seen"), bitsetofint::type(0)));

    net.put_value (pid_store, m);
  }

  we::type::transition_t tnet ( "tnet"
                              , net
                              , boost::none
                              , true
                              , we::type::property::type()
                              , we::priority_type()
                              );
  tnet.add_port
    (we::type::port_t ("vid", we::type::PORT_IN, std::string ("unsigned long"), pid_vid));
  tnet.add_port
    (we::type::port_t ("store", we::type::PORT_IN, sig_store, pid_store));
  we::port_id_type const port_id_store_out_net
    (tnet.add_port
      (we::type::port_t ("store", we::type::PORT_OUT, sig_store, pid_store))
    );
  we::port_id_type const port_id_pair_net
    (tnet.add_port
      (we::type::port_t ("pair", we::type::PORT_OUT, sig_pair, pid_pair))
    );

  we::type::activity_t act (tnet, boost::none);

  boost::mt19937 engine;

  if (act.transition().net())
  {
    while ( boost::optional<we::type::activity_t> sub
          = boost::get<we::type::net_type> (act.transition().data())
          . fire_expressions_and_extract_activity_random
              ( engine
              , [] (pnet::type::value::value_type const&, pnet::type::value::value_type const&)
                {
                  throw std::logic_error ("got unexpected workflow_response");
                }
              )
          )
    {
      exec_context ctxt;
      sub->execute (&ctxt);
      act.inject (*sub);
    }
  }

  std::unordered_map
    < we::port_id_type
    , std::list<pnet::type::value::value_type>
    > values_by_port_id;

  we::type::activity_t::output_t const output (act.output());

  for ( std::pair<pnet::type::value::value_type, we::port_id_type> const&
          token_on_port
      : output
      )
  {
    values_by_port_id[token_on_port.second].push_back (token_on_port.first);
  }

  BOOST_REQUIRE_EQUAL (values_by_port_id.size(), 2);
  BOOST_REQUIRE_EQUAL (values_by_port_id.at (port_id_store_out_net).size(), 2);
  BOOST_REQUIRE_EQUAL (values_by_port_id.at (port_id_pair_net).size(), 4);

  for (unsigned long bid (0); bid < 2; ++bid)
  {
    bitsetofint::type bs;

    for (unsigned long vid (0); vid < 2; ++vid)
    {
      pnet::type::value::value_type s;
      pnet::type::value::poke ("bid", s, bid);
      pnet::type::value::poke ("vid", s, vid);

      BOOST_REQUIRE
        ( std::find ( values_by_port_id.at (port_id_pair_net).begin()
                    , values_by_port_id.at (port_id_pair_net).end()
                    , s
                    )
        != values_by_port_id.at (port_id_pair_net).end()
        );

      bs.ins (vid);
    }

    pnet::type::value::value_type p;
    pnet::type::value::poke ("bid", p, bid);
    pnet::type::value::poke ("seen", p, bs);

    BOOST_REQUIRE
      ( std::find ( values_by_port_id.at (port_id_store_out_net).begin()
                  , values_by_port_id.at (port_id_store_out_net).end()
                  , p
                  )
      != values_by_port_id.at (port_id_store_out_net).end()
      );
  }
}
