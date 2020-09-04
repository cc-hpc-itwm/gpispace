// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type_map_type.hpp>
#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/type/signature.hpp>

#include <list>
#include <string>
#include <unordered_map>

#include <boost/filesystem.hpp>
#include <boost/variant.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct structure_type : with_position_of_definition
      {
      public:
        structure_type ( const util::position_type&
                       , const pnet::type::signature::structured_type& sig
                       );

        const pnet::type::signature::structured_type& signature() const;
        const std::string& name() const;

        void specialize (const std::unordered_map<std::string, std::string>&);
        void resolve (const std::unordered_map<std::string, structure_type>&);

      private:
        pnet::type::signature::structured_type _sig;
      };

      typedef std::list<structure_type> structs_type;

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const structure_type & st
                  );
      }
    }

    namespace structure_type_util
    {
      typedef std::unordered_map<std::string, type::structure_type> set_type;
      typedef std::unordered_map<std::string, std::string> forbidden_type;

      set_type make (const type::structs_type & structs, state::type const&);

      set_type join (set_type const& above, set_type const& below);
    }
  }
}
