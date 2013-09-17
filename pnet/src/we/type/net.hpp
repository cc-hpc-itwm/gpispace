// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_NET_HPP
#define _WE_TYPE_NET_HPP

#include <we/type/net.fwd.hpp>

#include <we/container/adjacency.hpp>
#include <we/container/priostore.hpp>
#include <we/serialize/unordered_map.hpp>
#include <we/type/connection.hpp>
#include <we/type/id.hpp>
#include <we/type/place.hpp>

#include <we/type/value.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/read.hpp>

#include <we/type/transition.hpp>

#include <we/util/cross.fwd.hpp>

#include <we/mgmt/type/activity.hpp>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/unordered_map.hpp>

#include <list>
#include <sstream>

namespace petri_net
{
  class net
  {
  public:
    //! \todo eliminate these, just do not copy or assign nets!
    net();
    net (const net&);
    net& operator= (const net&);

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
    void modify_transitions
      ( const boost::function<void ( const transition_id_type&
                                   , we::type::transition_t&
                                   )>&
      );

    void put_token (const place_id_type&, const pnet::type::value::value_type&);
    void put_value (const place_id_type&, const pnet::type::value::value_type&);

    const std::list<pnet::type::value::value_type>&
      get_token (const place_id_type&) const;

    void delete_all_token (const place_id_type&);
    bool can_fire() const;

    template<typename Engine>
    we::mgmt::type::activity_t extract_activity_random (Engine& engine)
    {
      return extract_activity (_enabled.random (engine));
    }

  private:
    place_id_type _place_id;
    boost::unordered_map<place_id_type,place::type> _pmap;

    transition_id_type _transition_id;
    boost::unordered_map<transition_id_type, we::type::transition_t> _tmap;

    adjacency::table<place_id_type,transition_id_type,connection_t> _adj_pt;
    adjacency::table<transition_id_type,place_id_type,connection_t> _adj_tp;

    typedef boost::unordered_map< place_id_type
                                , std::list<pnet::type::value::value_type>
                                > token_by_place_id_type;

    token_by_place_id_type _token_by_place_id;

    friend class boost::serialization::access;
    template<typename Archive>
    void save (Archive& ar, const unsigned int) const
    {
      ar & BOOST_SERIALIZATION_NVP (_place_id);
      ar & BOOST_SERIALIZATION_NVP (_pmap);
      ar & BOOST_SERIALIZATION_NVP (_transition_id);
      ar & BOOST_SERIALIZATION_NVP (_tmap);
      ar & BOOST_SERIALIZATION_NVP (_adj_pt);
      ar & BOOST_SERIALIZATION_NVP (_adj_tp);
      {
        const std::size_t s (_token_by_place_id.size());
        ar & s;
      }
      BOOST_FOREACH ( const token_by_place_id_type::value_type& pl
                    , _token_by_place_id
                    )
      {
        ar & pl.first;
        {
          const std::size_t s (pl.second.size());
          ar & s;
        }
        BOOST_FOREACH (const pnet::type::value::value_type& x, pl.second)
        {
          std::ostringstream oss;
          oss << pnet::type::value::show (x);
          const std::string token_rep (oss.str());
          ar & token_rep;
        }
      }
    }
    template<typename Archive>
    void load (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP (_place_id);
      ar & BOOST_SERIALIZATION_NVP (_pmap);
      ar & BOOST_SERIALIZATION_NVP (_transition_id);
      ar & BOOST_SERIALIZATION_NVP (_tmap);
      ar & BOOST_SERIALIZATION_NVP (_adj_pt);
      ar & BOOST_SERIALIZATION_NVP (_adj_tp);
      std::size_t token_by_place_id_size;
      ar & token_by_place_id_size;
      while (token_by_place_id_size --> 0)
      {
        place_id_type place_id;
        ar & place_id;
        token_by_place_id_type::iterator pos
          ( _token_by_place_id.insert
            (std::make_pair ( place_id
                            , std::list<pnet::type::value::value_type>()
                            )
            ).first
          );
        std::size_t num_tokens;
        ar & num_tokens;
        while (num_tokens --> 0)
        {
          std::string token_rep;
          ar & token_rep;
          pos->second.push_back (pnet::type::value::read (token_rep));
        }
      }

      BOOST_FOREACH ( const transition_id_type& tid
                    , _tmap | boost::adaptors::map_keys
                    )
      {
        update_enabled (tid);
      }
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    we::container::priority_store _enabled;

    boost::unordered_map
      < transition_id_type
      , boost::unordered_map< petri_net::place_id_type
                            , we::util::pos_and_distance_type
                            >
      > _enabled_choice;

    void get_enabled_choice (const net&);

    void update_enabled (const transition_id_type&);
    void update_enabled_put_token
      ( const transition_id_type&
      , const place_id_type&
      , const std::list<pnet::type::value::value_type>::iterator&
      );

    void disable (const transition_id_type&);

    we::mgmt::type::activity_t extract_activity (const transition_id_type&);
    void eval_cross (const transition_id_type&, we::util::cross_type&);
  };
}

#endif
