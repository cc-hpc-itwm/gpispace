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

#include <xml/parse/util/valid_name.hpp>

#include <xml/parse/error.hpp>

#include <fhg/util/cctype.hpp>

namespace xml
{
  namespace parse
  {
    //! \todo do not re-implement fhg::util::parse::require::identifier
    std::string parse_name (fhg::util::parse::position& pos)
    {
      std::string name;

      while (!pos.end() && fhg::util::isspace (*pos))
      {
        ++pos;
      }

      if (!pos.end() && (fhg::util::isalpha (*pos) || *pos == '_'))
      {
        name.push_back (*pos);

        ++pos;

        while (!pos.end() && (fhg::util::isalnum (*pos) || *pos == '_'))
        {
          name.push_back (*pos);

          ++pos;
        }
      }

      while (!pos.end() && fhg::util::isspace (*pos))
      {
        ++pos;
      }

      return name;
    }

    std::string validate_name ( std::string const& name
                              , std::string const& type
                              , ::boost::filesystem::path const& path
                              )
    {
      fhg::util::parse::position pos (name);

      if (parse_name (pos) != name)
      {
        throw error::invalid_name (name, type, path);
      }

      return name;
    }
  }
}
