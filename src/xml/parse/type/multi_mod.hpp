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

#include <unordered_map>

#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/preferences.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      using multi_module_map = std::unordered_map < preference_type
                                                  , module_type
                                                  >;

      struct multi_module_type
      {
      public:
        multi_module_type () = default;

        void add (module_type const& mod);

        multi_module_map const& modules() const;

        ::boost::optional<we::type::eureka_id_type> const& eureka_id() const;

      private:
        multi_module_map  _modules;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, multi_module_type const&);
      }
    }
  }
}
