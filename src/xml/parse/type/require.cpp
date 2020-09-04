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

#include <xml/parse/type/require.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      requirements_type::requirements_type()
        : _map (0)
      { }

      void requirements_type::set ( const require_key_type & key
                                  , const bool & mandatory
                                  )
      {
        map_type::iterator pos (_map.find (key));

        if (pos != _map.end())
        {
          pos->second |= mandatory;
        }
        else
        {
          _map[key] = mandatory;
        }
      }

      requirements_type::const_iterator requirements_type::begin () const
      {
        return _map.begin();
      }
      requirements_type::const_iterator requirements_type::end () const
      {
        return _map.end();
      }

      void requirements_type::join (const requirements_type& reqs)
      {
        for (const map_type::value_type& req : reqs)
        {
          set (req.first, req.second);
        }
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const requirements_type & cs
                  )
        {
          for ( requirements_type::const_iterator cap (cs.begin())
              ; cap != cs.end()
              ; ++cap
              )
          {
            s.open ("require");
            s.attr ("key", cap->first);
            s.attr ("mandatory", cap->second);
            s.close ();
          }
        }
      }
    }
  }
}
