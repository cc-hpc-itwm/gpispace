// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_NET_HPP
#define _WE_TYPE_NET_HPP

#include <we/type/net.fwd.hpp>

#include <we/type/activity.hpp>
#include <we/serialize/unordered_map.hpp>
#include <we/type/connection.hpp>
#include <we/type/id.hpp>
#include <we/type/place.hpp>
#include <we/type/transition.fwd.hpp>
#include <we/type/value.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/random.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/any_range.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/utility.hpp>

#include <list>
#include <sstream>

namespace we
{
  namespace type
  {
    class net_type
    {
    public:
      typedef boost::bimaps::bimap
        < boost::bimaps::unordered_multiset_of<transition_id_type>
        , boost::bimaps::unordered_multiset_of<place_id_type>
        , boost::bimaps::set_of_relation<>
        > adj_tp_type;
      typedef boost::bimaps::bimap
        < boost::bimaps::unordered_multiset_of<place_id_type>
        , boost::bimaps::unordered_multiset_of<transition_id_type>
        , boost::bimaps::set_of_relation<>
        > adj_pt_type;
      typedef boost::bimaps::bimap
        < port_id_type
        , place_id_type
        , boost::bimaps::left_based
        , boost::bimaps::with_info<we::type::property::type>
        > port_to_place_with_info_type;
      typedef boost::bimaps::bimap
        < place_id_type
        , port_id_type
        , boost::bimaps::left_based
        , boost::bimaps::with_info<we::type::property::type>
        > place_to_port_with_info_type;
      typedef boost::unordered_map< transition_id_type
                                  , port_to_place_with_info_type
                                  > port_to_place_type;
      typedef boost::unordered_map< transition_id_type
                                  , place_to_port_with_info_type
                                  > place_to_port_type;

      //! \todo eliminate these, just do not copy or assign nets!
      net_type();
      net_type (const net_type&);
      net_type& operator= (const net_type&);

      place_id_type add_place (const place::type&);
      transition_id_type add_transition (const we::type::transition_t&);

      const boost::unordered_map<place_id_type, place::type>& places() const;
      const boost::unordered_map<transition_id_type, we::type::transition_t>&
        transitions() const;

      void add_connection ( edge::type
                          , transition_id_type
                          , place_id_type
                          , port_id_type
                          , we::type::property::type const&
                          );

      adj_tp_type const& transition_to_place() const;
      adj_pt_type const& place_to_transition_consume() const;
      adj_pt_type const& place_to_transition_read() const;

      port_to_place_type const& port_to_place() const;
      place_to_port_type const& place_to_port() const;

      void put_value (place_id_type, const pnet::type::value::value_type&);

      const std::list<pnet::type::value::value_type>&
        get_token (place_id_type) const;

      template<typename Engine>
      boost::optional<we::type::activity_t>
      fire_expressions_and_extract_activity_random (Engine& engine)
      {
        while (!_enabled.empty())
        {
          boost::unordered_set<we::transition_id_type> const& transition_ids
            (_enabled.begin()->second);
          boost::uniform_int<std::size_t> random (0, transition_ids.size() - 1);
          transition_id_type const transition_id
            (*boost::next (transition_ids.begin(), random (engine)));
          we::type::transition_t const& transition (_tmap.at (transition_id));

          if (transition.expression())
          {
            fire_expression (transition_id, transition);
          }
          else
          {
            return extract_activity (transition_id, transition);
          }
        }

        return boost::none;
      }

    private:
      place_id_type _place_id;
      boost::unordered_map<place_id_type, place::type> _pmap;

      transition_id_type _transition_id;
      boost::unordered_map<transition_id_type, we::type::transition_t> _tmap;

      adj_pt_type _adj_pt_consume;
      adj_pt_type _adj_pt_read;
      adj_tp_type _adj_tp;

      port_to_place_type _port_to_place;
      place_to_port_type _place_to_port;

      typedef boost::unordered_map< place_id_type
                                  , std::list<pnet::type::value::value_type>
                                  > token_by_place_id_type;

      token_by_place_id_type _token_by_place_id;

      typedef std::map< we::priority_type
                      , boost::unordered_set<we::transition_id_type>
                      , std::greater<we::priority_type>
                      > enabled_type;

      enabled_type _enabled;

      typedef std::pair
        < std::list<pnet::type::value::value_type>::iterator
        , std::list<pnet::type::value::value_type>::iterator::difference_type
        > pos_and_distance_type;

      boost::unordered_map
        < transition_id_type
        , boost::unordered_map<place_id_type, pos_and_distance_type>
        > _enabled_choice;

      void get_enabled_choice (const net_type&);

      typedef boost::tuple
        < place_id_type
        , std::list<pnet::type::value::value_type>::iterator
        , std::list<pnet::type::value::value_type>::iterator::difference_type
        > to_be_updated_type;

      void update_enabled (transition_id_type);
      void update_enabled_put_token
        ( transition_id_type
        , to_be_updated_type const&
        );

      void disable (transition_id_type);

      we::type::activity_t extract_activity
        (transition_id_type, we::type::transition_t const&);
      void fire_expression (transition_id_type, we::type::transition_t const&);

      typedef std::pair< place_id_type
                       , std::list<pnet::type::value::value_type>::iterator
                       > token_to_be_deleted_type;

      std::list<token_to_be_deleted_type> do_extract
        ( transition_id_type
        , we::type::transition_t const&
        , boost::function
            <void (port_id_type, pnet::type::value::value_type const&)>
        ) const;
      void do_delete (std::list<token_to_be_deleted_type> const&);

      to_be_updated_type do_put_value
        (place_id_type, pnet::type::value::value_type const&);
      void do_update (to_be_updated_type const&);

      friend class boost::serialization::access;
      template<typename Archive>
      void save (Archive& ar, const unsigned int) const
      {
        ar & BOOST_SERIALIZATION_NVP (_place_id);
        ar & BOOST_SERIALIZATION_NVP (_pmap);
        ar & BOOST_SERIALIZATION_NVP (_transition_id);
        ar & BOOST_SERIALIZATION_NVP (_tmap);
        ar & BOOST_SERIALIZATION_NVP (_adj_pt_consume);
        ar & BOOST_SERIALIZATION_NVP (_adj_pt_read);
        ar & BOOST_SERIALIZATION_NVP (_adj_tp);
        ar & BOOST_SERIALIZATION_NVP (_port_to_place);
        ar & BOOST_SERIALIZATION_NVP (_place_to_port);
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
        ar & BOOST_SERIALIZATION_NVP (_adj_pt_consume);
        ar & BOOST_SERIALIZATION_NVP (_adj_pt_read);
        ar & BOOST_SERIALIZATION_NVP (_adj_tp);
        ar & BOOST_SERIALIZATION_NVP (_port_to_place);
        ar & BOOST_SERIALIZATION_NVP (_place_to_port);
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

        BOOST_FOREACH (transition_id_type tid, _tmap | boost::adaptors::map_keys)
        {
          update_enabled (tid);
        }
      }
      BOOST_SERIALIZATION_SPLIT_MEMBER()
    };
  }
}

#endif
