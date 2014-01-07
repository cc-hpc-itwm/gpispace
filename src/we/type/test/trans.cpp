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
#include <we/type/port.hpp>
#include <we/type/transition.hpp>
#include <we/type/id.hpp>

#include <we/type/net.hpp>
#include <we/type/signature.hpp>

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

  petri_net::place_id_type pid_pair (net.add_place (place::type("pair", sig_pair)));

  petri_net::port_id_type const port_id_vid
    ( trans_inner.add_port
      (we::type::port_t ("vid",we::type::PORT_IN,std::string("long")))
    );
  petri_net::port_id_type const port_id_store_out
    ( trans_inner.add_port
      (we::type::port_t ("store",we::type::PORT_OUT,sig_store))
    );
  petri_net::port_id_type const port_id_store_in
    ( trans_inner.add_port
      (we::type::port_t ("store",we::type::PORT_IN,sig_store))
    );
  petri_net::port_id_type const& port_id_pair
    ( trans_inner.add_port
      (we::type::port_t ("pair",we::type::PORT_OUT,sig_pair))
    );

  trans_inner.add_connection
    (pid_vid, port_id_vid, we::type::property::type());
  trans_inner.add_connection
    (pid_store, port_id_store_in, we::type::property::type());
  trans_inner.add_connection
    (port_id_pair, pid_pair, we::type::property::type());
  trans_inner.add_connection
    (port_id_store_out, pid_store, we::type::property::type());

  petri_net::transition_id_type tid (net.add_transition (trans_inner));

  net.add_connection (PT, tid, pid_store);
  net.add_connection (TP, tid, pid_store);
  net.add_connection (PT_READ, tid, pid_vid);
  net.add_connection (TP, tid, pid_pair);

  net.put_value (pid_vid, 0L);

  {
    pnet::type::value::structured_type m;

    m.push_back (std::make_pair (std::string ("bid"), 0L));
    m.push_back (std::make_pair (std::string ("seen"), bitsetofint::type(0)));

    net.put_value (pid_store, m);
  }
  // ************************************ //

  transition_t tnet ("tnet", net
                    , condition::type ("true")
                    , true, we::type::property::type()
                    );
  tnet.add_port
    (we::type::port_t ("vid", we::type::PORT_IN, std::string ("long"), pid_vid));
  tnet.add_port
    (we::type::port_t ("store", we::type::PORT_IN, sig_store, pid_store));
  tnet.add_port
    (we::type::port_t ("store", we::type::PORT_OUT, sig_store, pid_store));
  tnet.add_port
    (we::type::port_t ("pair", we::type::PORT_OUT, sig_pair, pid_pair));

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

  transition_t t1 ("t1", we::type::module_call_t ("m", "f")
                  , condition::type ("true"), false, we::type::property::type()
                  );

  petri_net::port_id_type const port_id_i_in
    ( t1.add_port
      (we::type::port_t ("i", we::type::PORT_IN, std::string ("long")))
    );
  petri_net::port_id_type const port_id_i_out
    ( t1.add_port
      (we::type::port_t ("i", we::type::PORT_OUT, std::string ("long")))
    );
  petri_net::port_id_type const port_id_max
    ( t1.add_port
      (we::type::port_t ("max", we::type::PORT_IN, std::string ("long")))
    );

  t1.add_connection
    (petri_net::place_id_type(0), port_id_i_in, we::type::property::type());
  t1.add_connection
    (petri_net::place_id_type(1), port_id_max, we::type::property::type());
  t1.add_connection
    (port_id_i_out, petri_net::place_id_type(0), we::type::property::type())
  ;

  std::cout << "i (inp) = " << t1.input_port_by_name ("i") << std::endl;
  std::cout << "max (inp) = " << t1.input_port_by_name ("max") << std::endl;
  std::cout << "i (out) = " << t1.output_port_by_name ("i") << std::endl;
  std::cout << "t1.p0 = " << t1.get_port (t1.input_port_by_name ("i")) << std::endl;

  transition_t t2 ("t2", we::type::expression_t ("true")
                  , condition::type ("true"), true, we::type::property::type()
                  );
  t2.add_port
    (we::type::port_t ("i", we::type::PORT_IN, std::string ("long")));
  t2.add_port
    (we::type::port_t ("sum", we::type::PORT_OUT, std::string ("long")));
  t2.add_port
    (we::type::port_t ("sum", we::type::PORT_IN, std::string ("long")));

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
