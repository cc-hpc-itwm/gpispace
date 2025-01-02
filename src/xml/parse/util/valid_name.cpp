// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
