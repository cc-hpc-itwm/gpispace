// Copyright (C) 2010-2016,2019-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/type/eureka.hpp>
#include <gspc/xml/parse/type/function.fwd.hpp>
#include <gspc/xml/parse/type/with_position_of_definition.hpp>
#include <gspc/xml/parse/type_map_type.hpp>

#include <gspc/xml/parse/util/position.fwd.hpp>

#include <gspc/util/xml.fwd.hpp>

#include <list>
#include <string>

#include <optional>



    namespace gspc::xml::parse::type
    {
      struct module_type : with_position_of_definition
      {
      public:
        module_type ( util::position_type const&
                    , std::string const& name
                    , std::string const& function
                    , std::optional<std::string> const& target
                    , std::optional<std::string> const& port_return
                    , std::list<std::string> const& port_arg
                    , std::optional<std::string> _memory_buffer_return
                    , std::list<std::string> _memory_buffer_arg
                    , std::optional<std::string> const& code
                    , std::optional<util::position_type> const& pod_of_code
                    , std::list<std::string> const& cincludes
                    , std::list<std::string> const& ldflags
                    , std::list<std::string> const& cxxflags
                    , std::optional<bool> const& pass_context
                    , std::optional<::gspc::we::type::eureka_id_type> const& eureka_id
                    , bool require_function_unloads_without_rest
                    , bool require_module_unloads_without_rest
                    );

        std::string const& name() const;
        std::string const& function() const;
        std::optional<std::string> const& target() const;
        std::optional<std::string> const& port_return() const;
        std::list<std::string> const& port_arg() const;
        std::optional<std::string> const& memory_buffer_return() const;
        std::list<std::string> const& memory_buffer_arg() const;
        std::optional<std::string> const& code() const;
        std::optional<util::position_type>
          position_of_definition_of_code() const;
        std::list<std::string> const& cincludes() const;
        std::list<std::string> const& ldflags() const;
        std::list<std::string> const& cxxflags() const;
        bool pass_context () const;
        std::optional<::gspc::we::type::eureka_id_type> const& eureka_id() const;
        bool require_function_unloads_without_rest() const;
        bool require_module_unloads_without_rest() const;

        bool operator== (module_type const&) const;

        void specialize (type_map_type const&);

      private:
        std::string _name;
        std::string _function;
        std::optional<std::string> _target;
        std::optional<std::string> _port_return;
        std::list<std::string> _port_arg;
        std::optional<std::string> _memory_buffer_return;
        std::list<std::string> _memory_buffer_arg;
        std::optional<std::string> _code;
        std::optional<util::position_type> _position_of_definition_of_code;
        std::list<std::string> _cincludes;
        std::list<std::string> _ldflags;
        std::list<std::string> _cxxflags;
        std::optional<bool> _pass_context;
        std::optional<::gspc::we::type::eureka_id_type> _eureka_id;
        bool _require_function_unloads_without_rest;
        bool _require_module_unloads_without_rest;
      };

      std::size_t hash_value (module_type const&);

      namespace dump
      {
        std::string dump_fun (module_type const&);

        void dump (::gspc::util::xml::xmlstream&, module_type const&);
      }
    }
