// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <we/type/net.fwd.hpp>

#include <we/type/activity.hpp>
#include <we/type/connection.hpp>
#include <we/type/id.hpp>
#include <we/type/place.hpp>
#include <we/type/transition.fwd.hpp>
#include <we/type/value.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>
#include <we/workflow_response.hpp>

#include <util-generic/serialization/std/unordered_map.hpp>

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <boost/random.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/any_range.hpp>
#include <boost/serialization/nvp.hpp>

#include <functional>
#include <iterator>
#include <list>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

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
        < port_id_type
        , std::string
        , boost::bimaps::left_based
        , boost::bimaps::with_info<we::type::property::type>
        > port_to_response_with_info_type;
      typedef boost::bimaps::bimap
        < place_id_type
        , port_id_type
        , boost::bimaps::left_based
        , boost::bimaps::with_info<we::type::property::type>
        > place_to_port_with_info_type;
      typedef std::unordered_map< transition_id_type
                                , port_to_place_with_info_type
                                > port_to_place_type;
      typedef std::unordered_map< transition_id_type
                                , port_to_response_with_info_type
                                > port_to_response_type;
      typedef std::unordered_map< transition_id_type
                                , place_to_port_with_info_type
                                > place_to_port_type;

      //! \todo eliminate these, just do not copy or assign nets!
      net_type();
      net_type (const net_type&);
      net_type& operator= (const net_type&);

      place_id_type add_place (const place::type&);
      transition_id_type add_transition (const we::type::transition_t&);

      const std::unordered_map<place_id_type, place::type>& places() const;
      const std::unordered_map<transition_id_type, we::type::transition_t>&
        transitions() const;

      void add_connection ( edge::type
                          , transition_id_type
                          , place_id_type
                          , port_id_type
                          , we::type::property::type const&
                          );
      void add_response ( transition_id_type
                        , port_id_type
                        , std::string const& to
                        , we::type::property::type const&
                        );

      adj_tp_type const& transition_to_place() const;
      adj_pt_type const& place_to_transition_consume() const;
      adj_pt_type const& place_to_transition_read() const;

      port_to_place_type const& port_to_place() const;
      port_to_response_type const& port_to_response() const;
      place_to_port_type const& place_to_port() const;

      void put_value (place_id_type, const pnet::type::value::value_type&);
      //! \note place must be marked with atribute put_token="true"
      void put_token (std::string place_name, pnet::type::value::value_type const&);

      const std::list<pnet::type::value::value_type>&
        get_token (place_id_type) const;

      template<typename Engine>
      boost::optional<we::type::activity_t>
      fire_expressions_and_extract_activity_random
        ( Engine& engine
        , we::workflow_response_callback const& workflow_response
        )
      {
        while (!_enabled.empty())
        {
          std::unordered_set<we::transition_id_type> const& transition_ids
            (_enabled.begin()->second);
          boost::uniform_int<std::size_t> random (0, transition_ids.size() - 1);
          transition_id_type const transition_id
            (*std::next (transition_ids.begin(), random (engine)));
          we::type::transition_t const& transition (_tmap.at (transition_id));

          if (transition.expression())
          {
            fire_expression (transition_id, transition, workflow_response);
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
      std::unordered_map<place_id_type, place::type> _pmap;
      std::unordered_map<std::string, place_id_type> _place_id_by_name;

      transition_id_type _transition_id;
      std::unordered_map<transition_id_type, we::type::transition_t> _tmap;

      adj_pt_type _adj_pt_consume;
      adj_pt_type _adj_pt_read;
      adj_tp_type _adj_tp;

      port_to_place_type _port_to_place;
      port_to_response_type _port_to_response;
      place_to_port_type _place_to_port;

      typedef std::unordered_map< place_id_type
                                , std::list<pnet::type::value::value_type>
                                > token_by_place_id_type;

      token_by_place_id_type _token_by_place_id;

      typedef std::map< we::priority_type
                      , std::unordered_set<we::transition_id_type>
                      , std::greater<we::priority_type>
                      > enabled_type;

      enabled_type _enabled;

      typedef std::pair
        < std::list<pnet::type::value::value_type>::iterator
        , std::list<pnet::type::value::value_type>::iterator::difference_type
        > pos_and_distance_type;

      std::unordered_map
        < transition_id_type
        , std::unordered_map<place_id_type, pos_and_distance_type>
        > _enabled_choice;

      void get_enabled_choice (const net_type&);

      typedef std::tuple
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
      void fire_expression
        ( transition_id_type
        , we::type::transition_t const&
        , we::workflow_response_callback const&
        );

      typedef std::pair< place_id_type
                       , std::list<pnet::type::value::value_type>::iterator
                       > token_to_be_deleted_type;

      std::list<token_to_be_deleted_type> do_extract
        ( transition_id_type
        , std::function
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
        ar & BOOST_SERIALIZATION_NVP (_place_id_by_name);
        ar & BOOST_SERIALIZATION_NVP (_transition_id);
        ar & BOOST_SERIALIZATION_NVP (_tmap);
        ar & BOOST_SERIALIZATION_NVP (_adj_pt_consume);
        ar & BOOST_SERIALIZATION_NVP (_adj_pt_read);
        ar & BOOST_SERIALIZATION_NVP (_adj_tp);
        ar & BOOST_SERIALIZATION_NVP (_port_to_place);
        ar & BOOST_SERIALIZATION_NVP (_port_to_response);
        ar & BOOST_SERIALIZATION_NVP (_place_to_port);
        {
          const std::size_t s (_token_by_place_id.size());
          ar & s;
        }
        for (const token_by_place_id_type::value_type& pl : _token_by_place_id)
        {
          ar & pl.first;
          {
            const std::size_t s (pl.second.size());
            ar & s;
          }
          for (const pnet::type::value::value_type& x : pl.second)
          {
            std::ostringstream oss;
            oss << pnet::type::value::show (x);
            const std::string token_rep (oss.str());
            ar & token_rep;
          }
        }

        std::size_t const number_of_enabled_transitions
          ( std::accumulate
            ( _enabled.begin(), _enabled.end()
            , 0
            , [](std::size_t n, enabled_type::value_type const& ps)
            {
              return n + ps.second.size();
            }
            )
          );

        ar & BOOST_SERIALIZATION_NVP (number_of_enabled_transitions);

        for ( std::unordered_set<transition_id_type> const& s
            : _enabled | boost::adaptors::map_values
            )
        {
          for (transition_id_type transition_id : s)
          {
            ar & transition_id;
          }
        }
      }
      template<typename Archive>
      void load (Archive& ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP (_place_id);
        ar & BOOST_SERIALIZATION_NVP (_pmap);
        ar & BOOST_SERIALIZATION_NVP (_place_id_by_name);
        ar & BOOST_SERIALIZATION_NVP (_transition_id);
        ar & BOOST_SERIALIZATION_NVP (_tmap);
        ar & BOOST_SERIALIZATION_NVP (_adj_pt_consume);
        ar & BOOST_SERIALIZATION_NVP (_adj_pt_read);
        ar & BOOST_SERIALIZATION_NVP (_adj_tp);
        ar & BOOST_SERIALIZATION_NVP (_port_to_place);
        ar & BOOST_SERIALIZATION_NVP (_port_to_response);
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
            pos->second.emplace_back (pnet::type::value::read (token_rep));
          }
        }

        std::size_t number_of_enabled_transitions;

        ar & number_of_enabled_transitions;

        std::unordered_set<transition_id_type> enabled_transitions;

        while (number_of_enabled_transitions --> 0)
        {
          transition_id_type transition_id;
          ar & transition_id;
          enabled_transitions.insert (transition_id);
        }

        for (transition_id_type tid : _tmap | boost::adaptors::map_keys)
        {
          if (enabled_transitions.count (tid) > 0)
          {
            update_enabled (tid);
          }
        }
      }
      BOOST_SERIALIZATION_SPLIT_MEMBER()
    };
  }
}
