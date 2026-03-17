// Copyright (C) 2010,2012-2016,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <test/we/net.common.hpp>
#include <gspc/we/type/Activity.hpp>
#include <gspc/we/type/net.hpp>
#include <gspc/we/type/value/poke.hpp>

BOOST_AUTO_TEST_CASE (create_and_execute_cross_product)
{
  gspc::we::type::net_type net;

  gspc::we::place_id_type const pid_vid
    (net.add_place (gspc::we::type::place::type ( "vid"
                                , std::string ("unsigned long")
                                , std::nullopt
                                , std::nullopt
                                , gspc::we::type::property::type{}
                                , gspc::we::type::place::type::Generator::No{}
                                )
                   )
    );

  gspc::pnet::type::signature::structure_type sig_store_fields;

  sig_store_fields.push_back
    (std::make_pair (std::string ("bid"), std::string ("unsigned long")));
  sig_store_fields.push_back
    (std::make_pair (std::string ("seen"), std::string ("bitset")));

  gspc::pnet::type::signature::structured_type const sig_store
    (std::make_pair (std::string ("store"), sig_store_fields));

  gspc::we::place_id_type const pid_store
    (net.add_place (gspc::we::type::place::type ( "store"
                                , sig_store
                                , std::nullopt
                                , std::nullopt
                                , gspc::we::type::property::type{}
                                , gspc::we::type::place::type::Generator::No{}
                                )
                   )
    );

  gspc::we::type::Transition trans_inner
    ( "trans_inner"
    , gspc::we::type::Expression
      ( "${store.seen} := bitset_insert (${store.seen}, ${vid});"
        "${store.bid}  := ${store.bid}                         ;"
        "${pair.bid}   := ${store.bid}                         ;"
        "${pair.vid}   := ${vid}                               "
      )
    , gspc::we::type::Expression ("!bitset_is_element (${store.seen}, ${vid})")
    , gspc::we::type::property::type()
    , gspc::we::priority_type()
    , std::optional<gspc::we::type::eureka_id_type>{}
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  gspc::pnet::type::signature::structure_type sig_pair_fields;

  sig_pair_fields.push_back
    (std::make_pair (std::string ("bid"), std::string ("unsigned long")));
  sig_pair_fields.push_back
    (std::make_pair (std::string ("vid"), std::string ("unsigned long")));

  gspc::pnet::type::signature::structured_type const sig_pair
    (std::make_pair (std::string ("pair"), sig_pair_fields));

  gspc::we::place_id_type const pid_pair
    (net.add_place (gspc::we::type::place::type ( "pair"
                                , sig_pair
                                , std::nullopt
                                , std::nullopt
                                , gspc::we::type::property::type{}
                                , gspc::we::type::place::type::Generator::No{}
                                )
                   )
    );

  gspc::we::port_id_type const port_id_vid
    ( trans_inner.add_port
        (gspc::we::type::Port ("vid", gspc::we::type::port::direction::In{}, std::string("unsigned long"), gspc::we::type::property::type{}))
    );
  gspc::we::port_id_type const port_id_store_out
    ( trans_inner.add_port
        (gspc::we::type::Port ("store", gspc::we::type::port::direction::Out{}, sig_store, gspc::we::type::property::type{}))
    );
  gspc::we::port_id_type const port_id_store_in
    ( trans_inner.add_port
        (gspc::we::type::Port ("store", gspc::we::type::port::direction::In{}, sig_store, gspc::we::type::property::type{}))
    );
  gspc::we::port_id_type const& port_id_pair
    ( trans_inner.add_port
        (gspc::we::type::Port ("pair", gspc::we::type::port::direction::Out{}, sig_pair, gspc::we::type::property::type{}))
    );

  gspc::we::transition_id_type const tid (net.add_transition (trans_inner));

  {
    gspc::we::type::property::type empty;

    net.add_connection (gspc::we::edge::PT{}, tid, pid_store, port_id_store_in, empty);
    net.add_connection (gspc::we::edge::TP{}, tid, pid_store, port_id_store_out, empty);
    net.add_connection (gspc::we::edge::PT_READ{}, tid, pid_vid, port_id_vid, empty);
    net.add_connection (gspc::we::edge::TP{}, tid, pid_pair, port_id_pair, empty);
  }

  net.put_value (pid_vid, 0UL);
  net.put_value (pid_vid, 1UL);

  {
    gspc::pnet::type::value::structured_type m;

    m.push_back (std::make_pair (std::string ("bid"), 0UL));
    m.push_back (std::make_pair (std::string ("seen"), gspc::pnet::type::bitsetofint::type(0)));

    net.put_value (pid_store, m);
  }

  {
    gspc::pnet::type::value::structured_type m;

    m.push_back (std::make_pair (std::string ("bid"), 1UL));
    m.push_back (std::make_pair (std::string ("seen"), gspc::pnet::type::bitsetofint::type(0)));

    net.put_value (pid_store, m);
  }

  gspc::we::type::Transition tnet ( "tnet"
                              , net
                              , std::nullopt
                              , gspc::we::type::property::type()
                              , gspc::we::priority_type()
                              , std::optional<gspc::we::type::eureka_id_type>{}
                              , std::list<gspc::we::type::Preference>{}
                              , gspc::we::type::track_shared{}
                              );
  tnet.add_port
    (gspc::we::type::Port ("vid", gspc::we::type::port::direction::In{}, std::string ("unsigned long"), pid_vid, gspc::we::type::property::type{}));
  tnet.add_port
    (gspc::we::type::Port ("store", gspc::we::type::port::direction::In{}, sig_store, pid_store, gspc::we::type::property::type{}));
  gspc::we::port_id_type const port_id_store_out_net
    (tnet.add_port
      (gspc::we::type::Port ("store", gspc::we::type::port::direction::Out{}, sig_store, pid_store, gspc::we::type::property::type{}))
    );
  gspc::we::port_id_type const port_id_pair_net
    (tnet.add_port
      (gspc::we::type::Port ("pair", gspc::we::type::port::direction::Out{}, sig_pair, pid_pair, gspc::we::type::property::type{}))
    );

  std::mt19937 engine;

    while ( net.fire_expressions_and_extract_activity_random_TESTING_ONLY
              ( engine
              , [] (gspc::pnet::type::value::value_type const&, gspc::pnet::type::value::value_type const&)
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
    < gspc::we::port_id_type
    , std::list<gspc::pnet::type::value::value_type>
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
    gspc::pnet::type::bitsetofint::type bs;

    for (unsigned long vid (0); vid < 2; ++vid)
    {
      gspc::pnet::type::value::value_type s;
      gspc::pnet::type::value::poke ("bid", s, bid);
      gspc::pnet::type::value::poke ("vid", s, vid);

      BOOST_REQUIRE
        ( std::find ( values_by_port_id.at (port_id_pair_net).begin()
                    , values_by_port_id.at (port_id_pair_net).end()
                    , s
                    )
        != values_by_port_id.at (port_id_pair_net).end()
        );

      bs.ins (vid);
    }

    gspc::pnet::type::value::value_type p;
    gspc::pnet::type::value::poke ("bid", p, bid);
    gspc::pnet::type::value::poke ("seen", p, bs);

    BOOST_REQUIRE
      ( std::find ( values_by_port_id.at (port_id_store_out_net).begin()
                  , values_by_port_id.at (port_id_store_out_net).end()
                  , p
                  )
      != values_by_port_id.at (port_id_store_out_net).end()
      );
  }
}
