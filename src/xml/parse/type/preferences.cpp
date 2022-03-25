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

#include <xml/parse/type/preferences.hpp>

#include <xml/parse/error.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      preferences_type::preferences_type
        (std::list<preference_type> targets)
        : _targets (std::move (targets))
      {}

      std::list<preference_type> const&
        preferences_type::targets() const
      {
        return _targets;
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream& s
                  , preferences_type const& cs
                  )
        {
          s.open ("preferences");

          for (auto const& pref_type : cs.targets())
          {
            s.open ("target");
            s.content (pref_type);
            s.close ();
          }

          s.close ();
        }
      }
    }
  }
}
