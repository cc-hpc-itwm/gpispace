// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <we/type/net.fwd.hpp>

#include <we/plugin/Plugins.hpp>
#include <we/type/activity.fwd.hpp>
#include <we/type/connection.hpp>
#include <we/type/id.hpp>
#include <we/type/place.hpp>
#include <we/type/transition.hpp>
#include <we/type/value.hpp>
#include <we/type/value/serialize.hpp>
#include <we/workflow_response.hpp>
#include <we/eureka_response.hpp>

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>

#include <forward_list>
#include <functional>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <cstdlib>

namespace we
{
  namespace type
  {
    [[noreturn]] void unexpected_eureka (eureka_ids_type const&);

    class net_type
    {
    public:
      using adj_tp_type
        = std::unordered_multimap<place_id_type, transition_id_type>;
      typedef boost::bimaps::bimap
        < boost::bimaps::unordered_multiset_of<place_id_type>
        , boost::bimaps::unordered_multiset_of<transition_id_type>
        , boost::bimaps::set_of_relation<>
        > adj_pt_type;
      typedef std::unordered_map
        < transition_id_type
        , std::unordered_map< port_id_type
                            , std::pair<place_id_type, we::type::property::type>
                            >
        > port_to_place_type;
      typedef std::unordered_map
        < transition_id_type
        , std::unordered_map< port_id_type
                            , std::pair<std::string, we::type::property::type>
                            >
        > port_to_response_type;
      typedef std::unordered_map
        < transition_id_type
        , port_id_type
        > port_to_eureka_type;
      typedef std::unordered_map
        < transition_id_type
        , std::unordered_map< place_id_type
                            , std::pair<port_id_type, we::type::property::type>
                            >
        > place_to_port_type;
      using token_by_id_type =
        std::unordered_map<token_id_type, pnet::type::value::value_type>;


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
      void add_eureka ( transition_id_type
                      , port_id_type
                      );

      adj_tp_type const& transition_to_place() const;
      adj_pt_type const& place_to_transition_consume() const;
      adj_pt_type const& place_to_transition_read() const;

      port_to_place_type const& port_to_place() const;
      port_to_place_type const& port_many_to_place() const;
      port_to_response_type const& port_to_response() const;
      port_to_eureka_type const& port_to_eureka() const;
      place_to_port_type const& place_to_port() const;

      void put_value (place_id_type, const pnet::type::value::value_type&);
      //! \note place must be marked with atribute put_token="true"
      void put_token
        (std::string place_name, pnet::type::value::value_type const&);

      token_by_id_type const& get_token (place_id_type) const;

      boost::optional<we::type::activity_t>
        fire_expressions_and_extract_activity_random
          ( std::mt19937&
          , we::workflow_response_callback const&
          , we::eureka_response_callback const&
          , gspc::we::plugin::Plugins&
          , gspc::we::plugin::PutToken
          );

      boost::optional<we::type::activity_t>
        fire_expressions_and_extract_activity_random
          ( std::mt19937&
          , we::workflow_response_callback const&
          , we::eureka_response_callback const& = &unexpected_eureka
          );

      void inject ( transition_id_type
                  , TokensOnPorts const& output
                  , TokensOnPorts const& input //! \todo remember input
                  , workflow_response_callback
                  = [] ( pnet::type::value::value_type const&
                       , pnet::type::value::value_type const&
                       ) {}
                  , eureka_response_callback = &unexpected_eureka
                  );

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
      port_to_place_type _port_many_to_place;
      port_to_eureka_type _port_to_eureka;
      port_to_response_type _port_to_response;
      place_to_port_type _place_to_port;

      typedef std::unordered_map< place_id_type
                                , token_by_id_type
                                > token_by_place_id_type;

      token_id_type _token_id;
      token_by_place_id_type _token_by_place_id;

      typedef std::map< we::priority_type
                      , std::unordered_set<we::transition_id_type>
                      , std::greater<we::priority_type>
                      > enabled_type;

      enabled_type _enabled;

      std::unordered_map
        < transition_id_type
        , std::unordered_map<place_id_type, std::pair<token_id_type, bool>>
        > _enabled_choice;

      typedef std::pair<place_id_type, token_id_type> to_be_updated_type;

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
        , we::eureka_response_callback const&
        , gspc::we::plugin::Plugins&
        , gspc::we::plugin::PutToken
        );

      typedef std::pair<place_id_type, token_id_type> token_to_be_deleted_type;

      std::forward_list<token_to_be_deleted_type> do_extract
        ( transition_id_type
        , std::function
            <void (port_id_type, pnet::type::value::value_type const&)>
        ) const;
      void do_delete (std::forward_list<token_to_be_deleted_type> const&);

      to_be_updated_type do_put_value
        (place_id_type, pnet::type::value::value_type const&);
      void do_update (to_be_updated_type const&);

      friend class boost::serialization::access;
      template<typename Archive>
      void serialize (Archive& ar, const unsigned int)
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
        ar & BOOST_SERIALIZATION_NVP (_port_many_to_place);
        ar & BOOST_SERIALIZATION_NVP (_port_to_response);
        ar & BOOST_SERIALIZATION_NVP (_port_to_eureka);
        ar & BOOST_SERIALIZATION_NVP (_place_to_port);
        ar & BOOST_SERIALIZATION_NVP (_token_id);
        ar & BOOST_SERIALIZATION_NVP (_token_by_place_id);
        ar & BOOST_SERIALIZATION_NVP (_enabled);
        ar & BOOST_SERIALIZATION_NVP (_enabled_choice);
      }
    };
  }
}
