// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/type/eureka.hpp>
#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/type_map_type.hpp>

#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <list>
#include <string>

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct module_type : with_position_of_definition
      {
      public:
        module_type ( util::position_type const&
                    , std::string const& name
                    , std::string const& function
                    , ::boost::optional<std::string> const& target
                    , ::boost::optional<std::string> const& port_return
                    , std::list<std::string> const& port_arg
                    , ::boost::optional<std::string> _memory_buffer_return
                    , std::list<std::string> _memory_buffer_arg
                    , ::boost::optional<std::string> const& code
                    , ::boost::optional<util::position_type> const& pod_of_code
                    , std::list<std::string> const& cincludes
                    , std::list<std::string> const& ldflags
                    , std::list<std::string> const& cxxflags
                    , ::boost::optional<bool> const& pass_context
                    , ::boost::optional<we::type::eureka_id_type> const& eureka_id
                    , bool require_function_unloads_without_rest
                    , bool require_module_unloads_without_rest
                    );

        std::string const& name() const;
        std::string const& function() const;
        ::boost::optional<std::string> const& target() const;
        ::boost::optional<std::string> const& port_return() const;
        std::list<std::string> const& port_arg() const;
        ::boost::optional<std::string> const& memory_buffer_return() const;
        std::list<std::string> const& memory_buffer_arg() const;
        ::boost::optional<std::string> const& code() const;
        ::boost::optional<util::position_type>
          position_of_definition_of_code() const;
        std::list<std::string> const& cincludes() const;
        std::list<std::string> const& ldflags() const;
        std::list<std::string> const& cxxflags() const;
        bool pass_context () const;
        ::boost::optional<we::type::eureka_id_type> const& eureka_id() const;
        bool require_function_unloads_without_rest() const;
        bool require_module_unloads_without_rest() const;

        bool operator== (module_type const&) const;

        void specialize (type_map_type const&);

      private:
        std::string _name;
        std::string _function;
        ::boost::optional<std::string> _target;
        ::boost::optional<std::string> _port_return;
        std::list<std::string> _port_arg;
        ::boost::optional<std::string> _memory_buffer_return;
        std::list<std::string> _memory_buffer_arg;
        ::boost::optional<std::string> _code;
        ::boost::optional<util::position_type> _position_of_definition_of_code;
        std::list<std::string> _cincludes;
        std::list<std::string> _ldflags;
        std::list<std::string> _cxxflags;
        ::boost::optional<bool> _pass_context;
        ::boost::optional<we::type::eureka_id_type> _eureka_id;
        bool _require_function_unloads_without_rest;
        bool _require_module_unloads_without_rest;
      };

      std::size_t hash_value (module_type const&);

      namespace dump
      {
        std::string dump_fun (module_type const&);

        void dump (::fhg::util::xml::xmlstream&, module_type const&);
      }
    }
  }
}
