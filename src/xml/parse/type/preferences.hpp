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
#include <list>
#include <unordered_map>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::string preference_type;

      class preferences_type
      {
        private:
          std::list<preference_type> _targets;

        public:
          preferences_type () = default;

          //! \note assumes targets is a list of unique names
          //! \note insertion order reflects preference ordering
          preferences_type (std::list<preference_type> targets);

          std::list<preference_type> const& targets() const;
      };

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream& s
                  , const preferences_type& cs
                  );
      }
    }
  }
}
