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

#include <xml/parse/util/validstructfield.hpp>

#include <xml/parse/util/valid_name.hpp>

#include <xml/parse/error.hpp>

#include <unordered_set>

namespace xml
{
  namespace parse
  {
    namespace
    {
      struct names_reserved
      {
      public:
        names_reserved()
          : _s()
        {
          _s.insert ("control");
          _s.insert ("bool");
          _s.insert ("int");
          _s.insert ("long");
          _s.insert ("uint");
          _s.insert ("ulong");
          _s.insert ("float");
          _s.insert ("double");
          _s.insert ("char");
        }

        bool operator() (const std::string& x) const
        {
          return _s.find (x) != _s.end();
        }

      private:
        std::unordered_set<std::string> _s;
      };

    }

    std::string validate_field_name ( const std::string& name
                                    , const boost::filesystem::path& path
                                    )
    {
      static names_reserved reserved;

      if (reserved (name))
      {
        throw error::invalid_field_name (name, path);
      }

      return validate_name (name, "fieldname", path);
    }
  }
}
