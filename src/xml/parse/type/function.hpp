// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/memory_buffer.hpp>
#include <xml/parse/type/memory_transfer.hpp>
#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/multi_mod.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/preferences.hpp>
#include <xml/parse/type/specialize.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/template.fwd.hpp>
#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/mk_fstream.hpp>
#include <xml/parse/util/position.fwd.hpp>
#include <xml/parse/util/unique.hpp>

#include <we/type/Port.hpp>
#include <we/type/Transition.hpp>
#include <we/type/property.hpp>

#include <boost/optional.hpp>

#include <string>
#include <unordered_set>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct function_type : with_position_of_definition
      {
      public:
        using unique_key_type = std::string;

        using ports_type = fhg::pnet::util::unique<port_type>;

        using content_type = ::boost::variant
          < expression_type
          , module_type
          , ::boost::recursive_wrapper<net_type>
          , multi_module_type
          >;

        // ***************************************************************** //

        function_type ( util::position_type const&
                      , ::boost::optional<std::string> const& name
                      , ports_type const& ports
                      , fhg::pnet::util::unique<memory_buffer_type> const&
                      , std::list<memory_get> const&
                      , std::list<memory_put> const&
                      , std::list<memory_getput> const&
                      , bool const& contains_a_module_call
                      , structs_type const& structs
                      , conditions_type const&
                      , requirements_type const& requirements
                      , preferences_type const& preferences
                      , content_type const& content
                      , we::type::property::type const& properties
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

        ::boost::optional<std::string> const& name() const;

        // ***************************************************************** //

        std::list<memory_get> const& memory_gets() const;
        std::list<memory_put> const& memory_puts() const;
        std::list<memory_getput> const& memory_getputs() const;

        fhg::pnet::util::unique<memory_buffer_type>
          const& memory_buffers() const;

        ports_type const& ports() const;

        ::boost::optional<port_type const&> get_port_in (std::string const& name) const;
        ::boost::optional<port_type const&> get_port_out (std::string const& name) const;

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
          (std::unordered_map<std::string, pnet::type::signature::signature_type> known);

        // ***************************************************************** //

        we::type::Transition synthesize
          ( std::string const&
          , state::type const&
          , std::unordered_map<std::string, we::port_id_type>& port_id_in
          , std::unordered_map<std::string, we::port_id_type>& port_id_out
          , conditions_type const&
          , we::type::property::type const&
          , requirements_type const&
          , we::priority_type
          , fhg::pnet::util::unique<place_map_type> const&
          , std::unordered_map<we::port_id_type, std::string>& real_place_names
          ) const;

        // ***************************************************************** //

        void specialize (state::type & state);

        void specialize ( type_map_type const& map
                        , type_get_type const& get
                        , xml::parse::structure_type_util::set_type const& known_structs
                        , state::type & state
                        );

        we::type::property::type const& properties() const;

        unique_key_type const& unique_key() const;

      private:
        //! \note should be const but can't be due to a massive amount
        //! of copy-assignments which should be copy-constructions but
        //! happen in optionals/variants and thus aren't. note that
        //! this can destroy the unique_key() when stored in a unique
        //! and directly modifying it (there is no setter, use
        //! with_name() instead)
        ::boost::optional<std::string> _name;

        ports_type _ports;
        fhg::pnet::util::unique<memory_buffer_type> _memory_buffers;

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
        we::type::property::type _properties;
      };

      // ***************************************************************** //

      struct fun_info_type
      {
        std::string name;
        std::string code;
        std::list<std::string> ldflags;
        std::list<std::string> cxxflags;
        ::boost::filesystem::path path;

        fun_info_type ( std::string const& _name
                      , std::string const& _code
                      , std::list<std::string> const& _ldflags
                      , std::list<std::string> const& _cxxflags
                      , ::boost::filesystem::path const& _path
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
        void dump ( ::fhg::util::xml::xmlstream & s
                  , function_type const& f
                  );
      }
    }
  }
}

namespace std
{
  template<> struct hash<xml::parse::type::fun_info_type>
  {
    size_t operator()(xml::parse::type::fun_info_type const&) const;
  };
}
