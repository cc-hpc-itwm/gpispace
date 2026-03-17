// Copyright (C) 2010-2016,2019-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>

#include <gspc/xml/parse/type/expression.hpp>
#include <gspc/xml/parse/type/memory_buffer.hpp>
#include <gspc/xml/parse/type/memory_transfer.hpp>
#include <gspc/xml/parse/type/mod.hpp>
#include <gspc/xml/parse/type/multi_mod.hpp>
#include <gspc/xml/parse/type/net.fwd.hpp>
#include <gspc/xml/parse/type/place_map.hpp>
#include <gspc/xml/parse/type/port.hpp>
#include <gspc/xml/parse/type/preferences.hpp>
#include <gspc/xml/parse/type/specialize.hpp>
#include <gspc/xml/parse/type/struct.hpp>
#include <gspc/xml/parse/type/template.fwd.hpp>
#include <gspc/xml/parse/type/transition.fwd.hpp>
#include <gspc/xml/parse/type/with_position_of_definition.hpp>
#include <gspc/xml/parse/util/mk_fstream.hpp>
#include <gspc/xml/parse/util/position.fwd.hpp>
#include <gspc/xml/parse/util/unique.hpp>

#include <gspc/we/type/Transition.hpp>

#include <gspc/we/type/Port.hpp>
#include <gspc/we/type/Transition.hpp>
#include <gspc/we/type/property.hpp>


#include <string>
#include <unordered_set>
#include <optional>



    namespace gspc::xml::parse::type
    {
      struct function_type : with_position_of_definition
      {
      public:
        using unique_key_type = std::string;

        using ports_type = fhgpnet::util::unique<port_type>;

        using content_type = ::boost::variant
          < expression_type
          , module_type
          , ::boost::recursive_wrapper<net_type>
          , multi_module_type
          >;

        // ***************************************************************** //

        function_type ( util::position_type const&
                      , std::optional<std::string> const& name
                      , ports_type const& ports
                      , fhgpnet::util::unique<memory_buffer_type> const&
                      , std::list<memory_get> const&
                      , std::list<memory_put> const&
                      , std::list<memory_getput> const&
                      , bool const& contains_a_module_call
                      , structs_type const& structs
                      , conditions_type const&
                      , requirements_type const& requirements
                      , preferences_type const& preferences
                      , content_type const& content
                      , ::gspc::we::type::property::type const& properties
                      );

        //! \note explicitly defaulted in compilation unit to be able
        //! to forward declare content_type only
        function_type (function_type const&);
        function_type (function_type&&);
        function_type& operator= (function_type const&);
        function_type& operator= (function_type&&);
        ~function_type();

        function_type with_name (std::string) const;

        content_type const& content() const;
        content_type& content();

        // ***************************************************************** //

        bool is_net() const;
        net_type net() const;

        // ***************************************************************** //

        std::optional<std::string> const& name() const;

        // ***************************************************************** //

        std::list<memory_get> const& memory_gets() const;
        std::list<memory_put> const& memory_puts() const;
        std::list<memory_getput> const& memory_getputs() const;

        fhgpnet::util::unique<memory_buffer_type>
          const& memory_buffers() const;

        ports_type const& ports() const;

        std::optional<std::reference_wrapper<port_type const>> get_port_in (std::string const& name) const;
        std::optional<std::reference_wrapper<port_type const>> get_port_out (std::string const& name) const;

        bool is_known_port_in (std::string const& name) const;
        bool is_known_port_out (std::string const& name) const;
        bool is_known_port (std::string const& name) const;
        bool is_known_port_inout (std::string const& name) const;
        bool is_known_tunnel (std::string const& name) const;

        // ***************************************************************** //

        conditions_type const& conditions() const;

        // ***************************************************************** //

        preferences_type const& preferences() const;

        // ***************************************************************** //

        void type_check (state::type const& state) const;
        void resolve_function_use_recursive
          (std::unordered_map<std::string, function_type const&> known);
        void resolve_types_recursive
          (std::unordered_map<std::string, gspc::pnet::type::signature::signature_type> known);

        // ***************************************************************** //

        ::gspc::we::type::Transition synthesize
          ( std::string const&
          , state::type const&
          , std::unordered_map<std::string, ::gspc::we::port_id_type>& port_id_in
          , std::unordered_map<std::string, ::gspc::we::port_id_type>& port_id_out
          , conditions_type const&
          , ::gspc::we::type::property::type const&
          , requirements_type const&
          , ::gspc::we::priority_type
          , fhgpnet::util::unique<place_map_type> const&
          , std::unordered_map<::gspc::we::port_id_type, std::string>& real_place_names
          , ::gspc::we::type::track_shared
          ) const;

        // ***************************************************************** //

        void specialize (state::type & state);

        void specialize ( type_map_type const& map
                        , type_get_type const& get
                        , gspc::xml::parse::structure_type_util::set_type const& known_structs
                        , state::type & state
                        );

        ::gspc::we::type::property::type const& properties() const;

        unique_key_type const& unique_key() const;

      private:
        //! \note should be const but can't be due to a massive amount
        //! of copy-assignments which should be copy-constructions but
        //! happen in optionals/variants and thus aren't. note that
        //! this can destroy the unique_key() when stored in a unique
        //! and directly modifying it (there is no setter, use
        //! with_name() instead)
        std::optional<std::string> _name;

        ports_type _ports;
        fhgpnet::util::unique<memory_buffer_type> _memory_buffers;

        std::list<memory_get> _memory_gets;
        std::list<memory_put> _memory_puts;
        std::list<memory_getput> _memory_getputs;

      public:
        bool contains_a_module_call;

        structs_type structs;

      private:
        conditions_type _conditions;
        preferences_type _preferences;

      public:
        requirements_type requirements;

      private:
        content_type _content;
        ::gspc::we::type::property::type _properties;
      };

      // ***************************************************************** //

      struct fun_info_type
      {
        std::string name;
        std::string code;
        std::list<std::string> ldflags;
        std::list<std::string> cxxflags;
        std::filesystem::path path;

        fun_info_type ( std::string const& _name
                      , std::string const& _code
                      , std::list<std::string> const& _ldflags
                      , std::list<std::string> const& _cxxflags
                      , std::filesystem::path const& _path
                      );

        bool operator== (fun_info_type const& other) const;
      };

      using fun_infos_type = std::unordered_set<fun_info_type>;

      using fun_info_map = std::unordered_map<std::string, fun_infos_type>;

      void mk_wrapper ( state::type const& state
                      , fun_info_map const& m
                      );

      void mk_makefile ( state::type const& state
                       , fun_info_map const& m
                       , std::unordered_set<std::string> const& structnames
                       );

      bool find_module_calls ( state::type const& state
                             , function_type&
                             , fun_info_map & m
                             );

      // ***************************************************************** //

      void struct_to_cpp ( state::type const&
                         , function_type const&
                         , std::unordered_set<std::string>&
                         );

      // ******************************************************************* //

      namespace dump
      {
        void dump ( ::gspc::util::xml::xmlstream & s
                  , function_type const& f
                  );
      }
    }



namespace std
{
  template<> struct hash<gspc::xml::parse::type::fun_info_type>
  {
    size_t operator()(gspc::xml::parse::type::fun_info_type const&) const;
  };
}
