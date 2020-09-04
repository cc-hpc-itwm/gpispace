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

#include <string>
#include <map>

#include <boost/optional.hpp>

namespace gpi
{
  namespace pc
  {
    class url_t
    {
    public:
      typedef std::map<std::string, std::string> arg_map_t;

      url_t (std::string const& type, std::string const& path)
        : m_type (type)
        , m_path (path)
      {}

      // url -> protocoll { '://' { host_path } { '?' { parameter_list }}}
      // protocoll -> identifier
      // host_path -> host_and_port { '/' path }
      // host_and_port -> host { ':' port }
      // host -> ip | identifier_with_dot_and_space | '*'
      // ip -> UINT8 '.' UINT8 '.' UINT8 '.' UINT8
      // port -> UINT16 | '*'
      // path -> identifier { '/' path }
      // parameter_list -> parameter { '&' parameter_list }
      // parameter -> key '=' value // throw on duplicate key
      // key -> identifier
      // value -> [a-zA-Z_0-9]+
      // identifier -> [a-zA-Z_][a-zA-Z_0-9]*
      // identifier_with_dot_and_space -> [a-zA-Z_][a-zA-Z_0-9. ]*
      explicit
      url_t (std::string const &);

      std::string const & type () const { return m_type; }
      std::string const & path () const { return m_path; }
      arg_map_t const &   args () const { return m_args; }

      void set (std::string const &k, std::string const &v)
      {
        m_args [k] = v;
      }

      boost::optional<std::string> get (std::string const&) const;
    private:
      std::string m_type;
      std::string m_path;
      arg_map_t   m_args;
    };
  }
}
