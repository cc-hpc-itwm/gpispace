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

#include <xml/parse/type/function.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/template.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <string>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <unordered_set>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct tmpl_type : with_position_of_definition
      {
      public:
        using unique_key_type = std::string;
        using names_type = std::unordered_set<std::string>;

        tmpl_type ( util::position_type const&
                  , ::boost::optional<std::string> const& name
                  , names_type const& tmpl_parameter
                  , function_type const& function
                  );

        ::boost::optional<std::string> const& name() const;
        names_type const& tmpl_parameter () const;

        function_type const& function() const;

        void resolve_function_use_recursive
          (std::unordered_map<std::string, function_type const&> known);
        void resolve_types_recursive
          (std::unordered_map<std::string, pnet::type::signature::signature_type> known);

        unique_key_type const& unique_key() const;

      private:
        ::boost::optional<std::string> const _name;
        names_type _tmpl_parameter;
        function_type _function;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, tmpl_type const&);
      }
    }
  }
}
