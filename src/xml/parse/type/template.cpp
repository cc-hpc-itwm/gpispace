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

#include <xml/parse/type/template.hpp>

#include <xml/parse/type/net.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      tmpl_type::tmpl_type
        ( const util::position_type& pod
        , const boost::optional<std::string>& name
        , const names_type& tmpl_parameter
        , function_type const& function
        )
          : with_position_of_definition (pod)
          , _name (name)
          , _tmpl_parameter (tmpl_parameter)
          , _function (function)
      {}

      const boost::optional<std::string>& tmpl_type::name() const
      {
        return _name;
      }

      const tmpl_type::names_type&
      tmpl_type::tmpl_parameter () const
      {
        return _tmpl_parameter;
      }

      function_type const& tmpl_type::function() const
      {
        return _function;
      }

      void tmpl_type::resolve_function_use_recursive
        (std::unordered_map<std::string, function_type const&> known)
      {
        _function.resolve_function_use_recursive (known);
      }

      void tmpl_type::resolve_types_recursive
        (std::unordered_map<std::string, pnet::type::signature::signature_type> known)
      {
        _function.resolve_types_recursive (known);
      }

      const tmpl_type::unique_key_type& tmpl_type::unique_key() const
      {
        //! \note Anonymous templates can't be stored in unique, thus
        //! just indirect.
        return *name();
      }

      namespace dump
      {
        void dump (fhg::util::xml::xmlstream & s, const tmpl_type & t)
        {
          s.open ("template");
          s.attr ("name", t.name());

          for (const std::string& tn : t.tmpl_parameter())
            {
              s.open ("template-parameter");
              s.attr ("type", tn);
              s.close ();
            }

          ::xml::parse::type::dump::dump (s, t.function());

          s.close ();
        }
      } // namespace dump
    }
  }
}
