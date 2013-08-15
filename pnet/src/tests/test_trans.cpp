/*
 * =====================================================================================
 *
 *       Filename:  test_trans.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  04/12/2010 12:18:50 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <we/type/expression.hpp>
#include <we/type/module_call.hpp>
#include <we/type/place.hpp>
#include <we/type/transition.hpp>
#include <we/type/id.hpp>

#include <we/type/net.hpp>
#include <we2/type/signature.hpp>

using petri_net::connection_t;
using petri_net::edge::PT;
using petri_net::edge::PT_READ;
using petri_net::edge::TP;

int main (int, char **)
{
  typedef we::type::transition_t transition_t;

  typedef petri_net::net pnet_t;

  // ************************************ //
  pnet_t net;

  petri_net::place_id_type pid_vid
    (net.add_place (place::type ("vid",std::string ("long"))));

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
      ( "${store.seen} := bitset_insert (${store.seen}, ${vid}); \
         ${store.bid}  := ${store.bid}                         ; \
         ${pair.bid}   := ${store.bid}                         ; \
         ${pair.vid}   := ${vid}                                 "
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
    ("vid",std::string("long"),we::type::PORT_IN);
  trans_inner.add_port
    ("store",sig_store,we::type::PORT_IN);
  trans_inner.add_port
    ("store",sig_store,we::type::PORT_OUT);
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

  net.put_value (pid_vid, 0L);

  {
    pnet::type::value::structured_type m;

    m.push_back (std::make_pair (std::string ("bid"), 0L));
    m.push_back (std::make_pair (std::string ("seen"), bitsetofint::type(0)));

    net.put_value (pid_store, m);
  }
  // ************************************ //

  transition_t tnet ("tnet", net);
  tnet.add_port
    ("vid", std::string ("long"), we::type::PORT_IN, pid_vid);
  tnet.add_port
    ("store", sig_store, we::type::PORT_IN, pid_store);
  tnet.add_port
    ("store", sig_store, we::type::PORT_OUT, pid_store);
  tnet.add_port
    ("pair", sig_pair, we::type::PORT_OUT, pid_pair)
  ;

  {
    std::ostringstream oss;
    {
      boost::archive::text_oarchive oa (oss, boost::archive::no_header);
      oa << BOOST_SERIALIZATION_NVP (tnet);
    }
    std::cout << "tnet (serialized): " << oss.str() << std::endl;

    transition_t tnet_d;
    {
      std::istringstream iss (oss.str());
      {
        boost::archive::text_iarchive ia (iss, boost::archive::no_header);
        ia >> BOOST_SERIALIZATION_NVP (tnet_d);
      }
    }
  }

  transition_t t1 ("t1", we::type::module_call_t ("m", "f"));

  t1.add_port
    ("i", std::string ("long"), we::type::PORT_IN);
  t1.add_port
    ("i", std::string ("long"), we::type::PORT_OUT);
  t1.add_port
    ("max", std::string ("long"), we::type::PORT_IN)
  ;

  t1.add_connection
    (petri_net::place_id_type(0), "i");
  t1.add_connection
    (petri_net::place_id_type(1), "max");
  t1.add_connection
    ("i", petri_net::place_id_type(0))
  ;

  std::cout << "i (inp) = " << t1.input_port_by_name ("i") << std::endl;
  std::cout << "max (inp) = " << t1.input_port_by_name ("max") << std::endl;
  std::cout << "i (out) = " << t1.output_port_by_name ("i") << std::endl;
  std::cout << "t1.p0 = " << t1.get_port (t1.input_port_by_name ("i")) << std::endl;

  transition_t t2 ("t2", we::type::expression_t ("true"));
  t2.add_port
    ("i", std::string ("long"), we::type::PORT_IN);
  t2.add_port
    ("sum", std::string ("long"), we::type::PORT_OUT);
  t2.add_port
    ("sum", std::string ("long"), we::type::PORT_IN)
  ;

  {
    std::ostringstream oss;
    {
      boost::archive::text_oarchive oa (oss, boost::archive::no_header);
      oa << BOOST_SERIALIZATION_NVP (t1);
    }
    std::cout << "t1 (serialized)=" << oss.str() << std::endl;
  }
  {
    std::ostringstream oss;
    {
      boost::archive::text_oarchive oa (oss, boost::archive::no_header);
      oa << BOOST_SERIALIZATION_NVP (t2);
    }
    std::cout << "t2 (serialized)=" << oss.str() << std::endl;
  }

  return EXIT_SUCCESS;
}
