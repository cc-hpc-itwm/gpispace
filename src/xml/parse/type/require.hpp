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

#include <fhg/util/xml.fwd.hpp>

#include <string>
#include <unordered_map>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::string require_key_type;

      struct requirements_type
      {
      private:
        typedef std::unordered_map<require_key_type,bool> map_type;

      public:
        requirements_type();

        void set ( const require_key_type & key
                 , const bool & mandatory = true
                 );

        typedef map_type::const_iterator const_iterator;

        const_iterator begin () const;
        const_iterator end () const;

        void join (const requirements_type& reqs);

      private:
        map_type _map;
      };

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const requirements_type & cs
                  );
      }
    }
  }
}
