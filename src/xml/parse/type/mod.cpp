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

#include <xml/parse/type/mod.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/util/valid_name.hpp>

#include <fhg/util/xml.hpp>

//! \todo remove, needed to make we::type::net_type a complete type
#include <we/type/net.hpp>

#include <fhg/assert.hpp>

#include <util-generic/print_container.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      module_type::module_type
        ( util::position_type const& pod
        , std::string const& name
        , std::string const& function
        , ::boost::optional<std::string> const& target
        , ::boost::optional<std::string> const& port_return
        , std::list<std::string> const& port_arg
        , ::boost::optional<std::string> memory_buffer_return
        , std::list<std::string> memory_buffer_arg
        , ::boost::optional<std::string> const& code
        , ::boost::optional<util::position_type> const& pod_of_code
        , std::list<std::string> const& cincludes
        , std::list<std::string> const& ldflags
        , std::list<std::string> const& cxxflags
        , ::boost::optional<bool> const& pass_context
        , ::boost::optional<we::type::eureka_id_type> const& eureka_id
        , bool require_function_unloads_without_rest
        , bool require_module_unloads_without_rest
        )
        : with_position_of_definition (pod)
        , _name (name)
        , _function (function)
        , _target (target)
        , _port_return (port_return)
        , _port_arg (port_arg)
        , _memory_buffer_return (memory_buffer_return)
        , _memory_buffer_arg (memory_buffer_arg)
        , _code (code)
        , _position_of_definition_of_code (pod_of_code)
        , _cincludes (cincludes)
        , _ldflags (ldflags)
        , _cxxflags (cxxflags)
        , _pass_context (pass_context)
        , _eureka_id (eureka_id)
        , _require_function_unloads_without_rest
            (require_function_unloads_without_rest)
        , _require_module_unloads_without_rest
            (require_module_unloads_without_rest)
      {
        fhg_assert (!(_port_return && _memory_buffer_return));

        //! \note enable unique module names with target
        //! \note to ensure unique namespace per-target-implemetation
        if (_target)
        {
          _name = _name + "_" + *_target;
        }
      }

      std::string const& module_type::name() const
      {
        return _name;
      }
      std::string const& module_type::function() const
      {
        return _function;
      }
      ::boost::optional<std::string> const& module_type::port_return() const
      {
        return _port_return;
      }
      std::list<std::string> const& module_type::port_arg() const
      {
        return _port_arg;
      }
      ::boost::optional<std::string> const&
        module_type::memory_buffer_return() const
      {
        return _memory_buffer_return;
      }
      std::list<std::string> const& module_type::memory_buffer_arg() const
      {
        return _memory_buffer_arg;
      }
      ::boost::optional<std::string> const& module_type::code() const
      {
        return _code;
      }
      const ::boost::optional<util::position_type>
        module_type::position_of_definition_of_code() const
      {
        return _position_of_definition_of_code;
      }
      std::list<std::string> const& module_type::cincludes() const
      {
        return _cincludes;
      }
      std::list<std::string> const& module_type::ldflags() const
      {
        return _ldflags;
      }
      std::list<std::string> const& module_type::cxxflags() const
      {
        return _cxxflags;
      }
      bool module_type::pass_context () const
      {
        return _pass_context ? *_pass_context : false;
      }
      ::boost::optional<std::string> const& module_type::target() const
      {
        return _target;
      }
      ::boost::optional<we::type::eureka_id_type> const& module_type::eureka_id() const
      {
        return _eureka_id;
      }
      bool module_type::require_function_unloads_without_rest() const
      {
        return _require_function_unloads_without_rest;
      }
      bool module_type::require_module_unloads_without_rest() const
      {
        return _require_module_unloads_without_rest;
      }

      bool module_type::operator == (module_type const& other) const
      {
        return _port_return == other._port_return
          && _port_arg == other._port_arg
          && _code == other._code
          && _cincludes == other._cincludes
          && _ldflags == other._ldflags
          && _cxxflags == other._cxxflags
          ;
      }

      void module_type::specialize (type_map_type const& type_map)
      {
        if (!type_map.empty())
        {
          _function += fhg::util::print_container
            ( "_", "_", "", type_map
            , [] (auto& s, auto const& p) -> decltype (s)
              {
                return s << p.first << "_" << p.second;
              }
            ).string()
            ;
        }
      }

      namespace dump
      {
        std::string dump_fun (module_type const& m)
        {
          std::ostringstream s;

          if (m.port_return())
            {
              s << *m.port_return() << " ";
            }

          s << m.function();

          s << " (";

          bool first (true);

          for (std::string const& arg : m.port_arg())
            {
              if (first)
              {
                first = false;
              }
              else
              {
                s << ", ";
              }

              s << arg;
            }

          s << ")";

          return s.str();
        }

        void dump (::fhg::util::xml::xmlstream& s, module_type const& m)
        {
          s.open ("module");
          s.attr ("name", m.name());
          s.attr ("function", dump_fun (m));
          s.attr ( "require_function_unloads_without_rest"
                 , m.require_function_unloads_without_rest()
                 );
          s.attr ( "require_module_unloads_without_rest"
                 , m.require_module_unloads_without_rest()
                 );

          for (std::string const& inc : m.cincludes())
            {
              s.open ("cinclude");
              s.attr ("href", inc);
              s.close ();
            }

          for (std::string const& flag : m.ldflags())
            {
              s.open ("ld");
              s.attr ("flag", flag);
              s.close ();
            }

          for (std::string const& flag : m.cxxflags())
            {
              s.open ("cxx");
              s.attr ("flag", flag);
              s.close ();
            }

          if (m.code())
            {
              s.open ("code");
              s.content ("<![CDATA[" + *m.code() + "]]>");
              s.close ();
            }

          s.close ();
        }
      }
    }
  }
}
