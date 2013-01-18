// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_NET_HPP
#define _WE_TYPE_NET_HPP

#include <we/type/net.fwd.hpp>

#include <we/container/adjacency.hpp>
#include <we/container/priostore.hpp>
#include <we/serialize/unordered_map.hpp>
#include <we/type/connection.hpp>
#include <we/type/id.hpp>
#include <we/type/token.hpp>
#include <we/type/place.hpp>
#include <we/util/cross.hpp>

#include <we/type/transition.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/unordered_map.hpp>

#include <vector>

namespace petri_net
{
  typedef std::pair<token::type, place_id_type> token_input_t;
  typedef std::vector<token_input_t> input_t;

  struct activity_t
  {
  public:
    const transition_id_type tid;
    const input_t input;

    activity_t (const transition_id_type _tid, const input_t& _input)
      : tid (_tid)
      , input (_input)
    {}
  };

  class net
  {
  public:
    place_id_type add_place (const place::type&);
    transition_id_type add_transition (const we::type::transition_t&);
    void add_connection (const connection_t&);

    void set_transition_priority ( const transition_id_type&
                                 , const priority_type&
                                 );
    priority_type get_transition_priority (const transition_id_type&) const;

    const place::type& get_place (const place_id_type&) const;
    const we::type::transition_t& get_transition (const transition_id_type&) const;

    const boost::unordered_map<place_id_type,place::type>& places() const;
    const boost::unordered_map<transition_id_type,we::type::transition_t>&
    transitions() const;
    const boost::unordered_set<connection_t> connections() const;

    const boost::unordered_map<place_id_type, connection_t>&
    out_of_transition (const transition_id_type&) const;
    const boost::unordered_map<place_id_type, connection_t>&
    in_to_transition (const transition_id_type&) const;
    const boost::unordered_map<transition_id_type, connection_t>&
    out_of_place (const place_id_type&) const;
    const boost::unordered_map<transition_id_type, connection_t>&
    in_to_place (const place_id_type&) const;

    connection_t get_connection_out ( const transition_id_type&
                                    , const place_id_type&
                                    ) const;
    connection_t get_connection_in ( const transition_id_type&
                                   , const place_id_type&
                                   ) const;
    bool is_read_connection ( const transition_id_type&
                            , const place_id_type&
                            ) const;

    void delete_edge_out ( const transition_id_type&
                         , const place_id_type&
                         );
    void delete_edge_in ( const transition_id_type&
                        , const place_id_type&
                        );

    void delete_place (const place_id_type&);
    void delete_transition (const transition_id_type&);

    place_id_type modify_place (const place_id_type&, const place::type&);
    transition_id_type modify_transition ( const transition_id_type&
                                         , const we::type::transition_t&
                                         );

    void put_token (const place_id_type&, const token::type&);
    void put_token (const place_id_type&);

    const std::vector<token::type>& get_token (const place_id_type&) const;

    void delete_all_token (const place_id_type&);
    bool can_fire() const;

    template<typename Engine>
    activity_t extract_activity_random (Engine& engine)
    {
      return extract_activity (_enabled.random (engine));
    }

  private:
    typedef boost::unordered_map< place_id_type
                                , std::vector<token::type>
                                > token_by_place_id_t;

    typedef cross::Traits<token_by_place_id_t>::vec_t choice_vec_t;
    typedef boost::unordered_map<transition_id_type, choice_vec_t> enabled_choice_t;
    typedef enabled_choice_t::iterator choice_iterator_t;

    place_id_type _place_id;
    boost::unordered_map<place_id_type,place::type> _pmap;

    transition_id_type _transition_id;
    boost::unordered_map<transition_id_type, we::type::transition_t> _tmap;

    adjacency::table<place_id_type,transition_id_type,connection_t> _adj_pt;
    adjacency::table<transition_id_type,place_id_type,connection_t> _adj_tp;

    token_by_place_id_t _token_by_place_id;

    priostore::type<transition_id_type> _enabled;

    enabled_choice_t _enabled_choice_consume;
    enabled_choice_t _enabled_choice_read;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(_place_id);
      ar & BOOST_SERIALIZATION_NVP(_pmap);
      ar & BOOST_SERIALIZATION_NVP(_transition_id);
      ar & BOOST_SERIALIZATION_NVP(_tmap);
      ar & BOOST_SERIALIZATION_NVP(_adj_pt);
      ar & BOOST_SERIALIZATION_NVP(_adj_tp);
      ar & BOOST_SERIALIZATION_NVP(_token_by_place_id);
      ar & BOOST_SERIALIZATION_NVP(_enabled);
      ar & BOOST_SERIALIZATION_NVP(_enabled_choice_consume);
      ar & BOOST_SERIALIZATION_NVP(_enabled_choice_read);
    }

    void update_enabled (const transition_id_type&);
    activity_t extract_activity (const transition_id_type&);
  };
}

#endif
