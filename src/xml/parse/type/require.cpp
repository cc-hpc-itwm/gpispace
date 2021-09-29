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

#include <xml/parse/type/require.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      void requirements_type::set (require_key_type const& key)
      {
        _set.insert (key);
      }

      requirements_type::const_iterator requirements_type::begin () const
      {
        return _set.begin();
      }
      requirements_type::const_iterator requirements_type::end () const
      {
        return _set.end();
      }

      void requirements_type::join (requirements_type const& reqs)
      {
        for (auto const& req : reqs)
        {
          set (req);
        }
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , requirements_type const& cs
                  )
        {
          for (auto const& cap : cs)
          {
            s.open ("require");
            s.attr ("key", cap);
            s.close ();
          }
        }
      }
    }
  }
}
