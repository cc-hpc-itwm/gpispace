// Copyright (C) 2012-2014,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/util/valid_name.hpp>

#include <gspc/xml/parse/error.hpp>

#include <gspc/util/cctype.hpp>


  namespace gspc::xml::parse
  {
    //! \todo do not re-implement gspc::util::parse::require::identifier
    std::string parse_name (gspc::util::parse::position& pos)
    {
      std::string name;

      while (!pos.end() && gspc::util::isspace (*pos))
      {
        ++pos;
      }

      if (!pos.end() && (gspc::util::isalpha (*pos) || *pos == '_'))
      {
        name.push_back (*pos);

        ++pos;

        while (!pos.end() && (gspc::util::isalnum (*pos) || *pos == '_'))
        {
          name.push_back (*pos);

          ++pos;
        }
      }

      while (!pos.end() && gspc::util::isspace (*pos))
      {
        ++pos;
      }

      return name;
    }

    std::string validate_name ( std::string const& name
                              , std::string const& type
                              , std::filesystem::path const& path
                              )
    {
      gspc::util::parse::position pos (name);

      if (parse_name (pos) != name)
      {
        throw error::invalid_name (name, type, path);
      }

      return name;
    }
  }
