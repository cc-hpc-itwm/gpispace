// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/net.fwd.hpp>

#include <we/eureka_response.hpp>
#include <we/plugin/Plugins.hpp>
#include <we/type/Activity.fwd.hpp>
#include <we/type/Transition.hpp>
#include <we/type/id.hpp>
#include <we/type/place.hpp>
#include <we/type/value.hpp>
#include <we/type/value/serialize.hpp>
#include <we/workflow_response.hpp>

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/variant.hpp>

#include <cstdlib>
#include <forward_list>
#include <functional>
#include <iosfwd>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace we
{
  namespace edge
  {
    struct PT{};
    struct PT_READ{};
    struct PT_NUMBER_OF_TOKENS{};
    struct TP{};
    struct TP_MANY{};

    //! \todo eliminate this, instead use subclasses of connection
    using type = ::boost::variant<PT, PT_READ, PT_NUMBER_OF_TOKENS, TP, TP_MANY>;

    bool is_incoming (type const&);

    std::ostream& operator<< (std::ostream&, PT const&);
    std::ostream& operator<< (std::ostream&, PT_READ const&);
    std::ostream& operator<< (std::ostream&, PT_NUMBER_OF_TOKENS const&);
    std::ostream& operator<< (std::ostream&, TP const&);
    std::ostream& operator<< (std::ostream&, TP_MANY const&);
  }

  namespace type
  {
    class net_type
    {
    public:
      using adj_tp_type
        = std::unordered_multimap<place_id_type, transition_id_type>;
      using adj_pt_type = boost::bimaps::bimap
        < ::boost::bimaps::unordered_multiset_of<place_id_type, std::hash<place_id_type>>
        , ::boost::bimaps::unordered_multiset_of<transition_id_type, std::hash<transition_id_type>>
        , ::boost::bimaps::set_of_relation<>
        >;

      struct PlaceIDWithProperty
      {
        place_id_type _place_id;
        we::type::property::type _property;

        template<typename Archive>
          auto serialize (Archive& ar, unsigned int) -> void
        {
          ar & _place_id & _property;
        }
      };

      using port_to_place_type = std::unordered_map
        < transition_id_type
        , std::unordered_map<port_id_type, PlaceIDWithProperty>
        >;

      struct ResponseWithProperty
      {
        std::string _to;
        we::type::property::type _property;

        template<typename Archive>
          auto serialize (Archive& ar, unsigned int) -> void
        {
          ar & _to & _property;
        }
      };

      using port_to_response_type = std::unordered_map
        < transition_id_type
        , std::unordered_map<port_id_type, ResponseWithProperty>
        >;
      using port_to_eureka_type = std::unordered_map
        < transition_id_type
        , port_id_type
        >;

      struct PortIDWithProperty
      {
        port_id_type _port_id;
        we::type::property::type _property;

        template<typename Archive>
          auto serialize (Archive& ar, unsigned int) -> void
        {
          ar & _port_id & _property;
        }
      };

      using place_to_port_type = std::unordered_map
        < transition_id_type
        , std::unordered_map<place_id_type, PortIDWithProperty>
        >;
      using token_by_id_type =
        std::unordered_map<token_id_type, pnet::type::value::value_type>;

      struct SelectedToken
      {
        token_id_type _id;
        bool _is_read;

        template<typename Archive>
          auto serialize (Archive& ar, unsigned long) -> void
        {
          ar & _id & _is_read;
        }
      };

      place_id_type add_place (place::type const&);
      transition_id_type add_transition (we::type::Transition const&);

      const std::unordered_map<place_id_type, place::type>& places() const;
      const std::unordered_map<transition_id_type, we::type::Transition>&
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

      void put_value (place_id_type, pnet::type::value::value_type const&);
      //! \note place must be marked with atribute put_token="true"
      void put_token
        (std::string place_name, pnet::type::value::value_type const&);

      token_by_id_type const& get_token (place_id_type) const;

      ::boost::optional<we::type::Activity>
        fire_expressions_and_extract_activity_random
          ( std::mt19937&
          , we::workflow_response_callback const&
          , we::eureka_response_callback const&
          , gspc::we::plugin::Plugins&
          , gspc::we::plugin::PutToken
          );

      ::boost::optional<we::type::Activity>
        fire_expressions_and_extract_activity_random_TESTING_ONLY
          ( std::mt19937&
          , we::workflow_response_callback const&
          , we::eureka_response_callback const&
          );

      void inject ( transition_id_type
                  , TokensOnPorts const& output
                  , TokensOnPorts const& input //! \todo remember input
                  , workflow_response_callback
                  , eureka_response_callback
                  );


      net_type& assert_correct_expression_types();

      bool might_use_virtual_memory() const;
      bool might_have_tasks_requiring_multiple_workers() const;
      bool might_use_modules_with_multiple_implementations() const;

      [[nodiscard]] auto number_of_tokens_place
        ( place_id_type
        ) const noexcept -> std::optional<place_id_type>
        ;

    private:
      place_id_type _place_id;
      std::unordered_map<place_id_type, place::type> _pmap;
      std::unordered_map<std::string, place_id_type> _place_id_by_name;

      std::unordered_map<place_id_type, place_id_type> _number_of_tokens_place;
      std::unordered_map
        < transition_id_type
        , std::unordered_map<port_id_type, place_id_type>
        > _consumed_from_places_by_port;

      [[nodiscard]] auto add_number_of_tokens_place
        ( place_id_type
        ) -> place_id_type
        ;
      auto increment_number_of_tokens (place_id_type) -> void;
      auto decrement_number_of_tokens (place_id_type) -> void;
      template<typename Update>
        auto update_number_of_tokens (place_id_type, Update&&) -> void;

      transition_id_type _transition_id;
      std::unordered_map<transition_id_type, we::type::Transition> _tmap;

      adj_pt_type _adj_pt_consume;
      adj_pt_type _adj_pt_read;
      adj_tp_type _adj_tp;

      port_to_place_type _port_to_place;
      port_to_place_type _port_many_to_place;
      port_to_eureka_type _port_to_eureka;
      port_to_response_type _port_to_response;
      place_to_port_type _place_to_port;

      using token_by_place_id_type =
        std::unordered_map<place_id_type, token_by_id_type>;

      token_id_type _token_id;
      token_by_place_id_type _token_by_place_id;

      using enabled_type =
        std::map< we::priority_type
                , std::unordered_set<we::transition_id_type>
                , std::greater<>
                >;

      enabled_type _enabled;

      std::unordered_map
        < transition_id_type
        , std::unordered_map<place_id_type, SelectedToken>
        > _enabled_choice;

      struct ToBeUpdated
      {
        place_id_type _place_id;
        token_id_type _token_id;
      };

      void update_enabled (transition_id_type);
      void update_enabled_put_token (transition_id_type, ToBeUpdated const&);

      void disable (transition_id_type);

      we::type::Activity extract_activity
        (transition_id_type, we::type::Transition const&);
      void fire_expression
        ( transition_id_type
        , we::type::Transition const&
        , we::workflow_response_callback const&
        , we::eureka_response_callback const&
        , gspc::we::plugin::Plugins&
        , gspc::we::plugin::PutToken
        );

      struct ToBeDeleted
      {
        place_id_type _place_id;
        token_id_type _token_id;
      };

      std::forward_list<ToBeDeleted> do_extract
        ( transition_id_type
        , std::function
            <void (port_id_type, pnet::type::value::value_type const&)>
        ) const;
      void do_delete (std::forward_list<ToBeDeleted> const&);

      ToBeUpdated do_put_value
        (place_id_type, pnet::type::value::value_type const&);
      void do_update (ToBeUpdated const&);

      friend class ::boost::serialization::access;
      template<typename Archive>
      void serialize (Archive& ar, unsigned int)
      {
        ar & _place_id;
        ar & _pmap;
        ar & _place_id_by_name;
        ar & _number_of_tokens_place;
        ar & _consumed_from_places_by_port;
        ar & _transition_id;
        ar & _tmap;
        ar & _adj_pt_consume;
        ar & _adj_pt_read;
        ar & _adj_tp;
        ar & _port_to_place;
        ar & _port_many_to_place;
        ar & _port_to_eureka;
        ar & _port_to_response;
        ar & _place_to_port;
        ar & _token_id;
        ar & _token_by_place_id;
        ar & _enabled;
        ar & _enabled_choice;
      }
    };
  }
}
