// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <we/test/net.common.hpp>
#include <we/type/Activity.hpp>
#include <we/type/net.hpp>
#include <we/type/value/poke.hpp>

BOOST_AUTO_TEST_CASE (create_and_execute_cross_product)
{
  we::type::net_type net;

  we::place_id_type const pid_vid
    (net.add_place (place::type ( "vid"
                                , std::string ("unsigned long")
                                , ::boost::none
                                , we::type::property::type{}
                                )
                   )
    );

  pnet::type::signature::structure_type sig_store_fields;

  sig_store_fields.push_back
    (std::make_pair (std::string ("bid"), std::string ("unsigned long")));
  sig_store_fields.push_back
    (std::make_pair (std::string ("seen"), std::string ("bitset")));

  pnet::type::signature::structured_type const sig_store
    (std::make_pair (std::string ("store"), sig_store_fields));

  we::place_id_type const pid_store
    (net.add_place (place::type ( "store"
                                , sig_store
                                , ::boost::none
                                , we::type::property::type{}
                                )
                   )
    );

  we::type::Transition trans_inner
    ( "trans_inner"
    , we::type::Expression
      ( "${store.seen} := bitset_insert (${store.seen}, ${vid});"
        "${store.bid}  := ${store.bid}                         ;"
        "${pair.bid}   := ${store.bid}                         ;"
        "${pair.vid}   := ${vid}                               "
      )
    , we::type::Expression ("!bitset_is_element (${store.seen}, ${vid})")
    , we::type::property::type()
    , we::priority_type()
    , ::boost::optional<we::type::eureka_id_type>{}
    , std::list<we::type::Preference>{}
    );

  pnet::type::signature::structure_type sig_pair_fields;

  sig_pair_fields.push_back
    (std::make_pair (std::string ("bid"), std::string ("unsigned long")));
  sig_pair_fields.push_back
    (std::make_pair (std::string ("vid"), std::string ("unsigned long")));

  pnet::type::signature::structured_type const sig_pair
    (std::make_pair (std::string ("pair"), sig_pair_fields));

  we::place_id_type const pid_pair
    (net.add_place (place::type ( "pair"
                                , sig_pair
                                , ::boost::none
                                , we::type::property::type{}
                                )
                   )
    );

  we::port_id_type const port_id_vid
    ( trans_inner.add_port
        (we::type::Port ("vid", we::type::port::direction::In{}, std::string("unsigned long"), we::type::property::type{}))
    );
  we::port_id_type const port_id_store_out
    ( trans_inner.add_port
        (we::type::Port ("store", we::type::port::direction::Out{}, sig_store, we::type::property::type{}))
    );
  we::port_id_type const port_id_store_in
    ( trans_inner.add_port
        (we::type::Port ("store", we::type::port::direction::In{}, sig_store, we::type::property::type{}))
    );
  we::port_id_type const& port_id_pair
    ( trans_inner.add_port
        (we::type::Port ("pair", we::type::port::direction::Out{}, sig_pair, we::type::property::type{}))
    );

  we::transition_id_type const tid (net.add_transition (trans_inner));

  {
    we::type::property::type empty;

    net.add_connection (we::edge::PT{}, tid, pid_store, port_id_store_in, empty);
    net.add_connection (we::edge::TP{}, tid, pid_store, port_id_store_out, empty);
    net.add_connection (we::edge::PT_READ{}, tid, pid_vid, port_id_vid, empty);
    net.add_connection (we::edge::TP{}, tid, pid_pair, port_id_pair, empty);
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

  we::type::Transition tnet ( "tnet"
                              , net
                              , ::boost::none
                              , we::type::property::type()
                              , we::priority_type()
                              , ::boost::optional<we::type::eureka_id_type>{}
                              , std::list<we::type::Preference>{}
                              );
  tnet.add_port
    (we::type::Port ("vid", we::type::port::direction::In{}, std::string ("unsigned long"), pid_vid, we::type::property::type{}));
  tnet.add_port
    (we::type::Port ("store", we::type::port::direction::In{}, sig_store, pid_store, we::type::property::type{}));
  we::port_id_type const port_id_store_out_net
    (tnet.add_port
      (we::type::Port ("store", we::type::port::direction::Out{}, sig_store, pid_store, we::type::property::type{}))
    );
  we::port_id_type const port_id_pair_net
    (tnet.add_port
      (we::type::Port ("pair", we::type::port::direction::Out{}, sig_pair, pid_pair, we::type::property::type{}))
    );

  std::mt19937 engine;

    while ( net.fire_expressions_and_extract_activity_random_TESTING_ONLY
              ( engine
              , [] (pnet::type::value::value_type const&, pnet::type::value::value_type const&)
                {
                  throw std::logic_error ("got unexpected workflow_response");
                }
              , unexpected_eureka
              )
          )
    {
      BOOST_FAIL ("BUMMER: no sub activity shall be created");
    }

  std::unordered_map
    < we::port_id_type
    , std::list<pnet::type::value::value_type>
    > values_by_port_id;

  for (auto const& [_ignore, token] : net.get_token (pid_store))
  {
    values_by_port_id[port_id_store_out_net].push_back (token);
  }
  for (auto const& [_ignore, token] : net.get_token (pid_pair))
  {
    values_by_port_id[port_id_pair_net].push_back (token);
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
