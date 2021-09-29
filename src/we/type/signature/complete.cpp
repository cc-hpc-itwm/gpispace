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

#include <we/type/signature/complete.hpp>

#include <we/type/value/name.hpp>

#include <iostream>
#include <unordered_map>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        typedef std::unordered_map<std::string,std::string> map_type;

        map_type init_typenames_complete()
        {
          map_type tn;

          tn[value::CONTROL()] = "we::type::literal::control";
          tn[value::BOOL()] = "bool";
          tn[value::INT()] = "int";
          tn[value::LONG()] = "long";
          tn[value::UINT()] = "unsigned int";
          tn[value::ULONG()] = "unsigned long";
          tn[value::FLOAT()] = "float";
          tn[value::DOUBLE()] = "double";
          tn[value::CHAR()] = "char";
          tn[value::STRING()] = "std::string";
          tn[value::BITSET()] = "bitsetofint::type";
          tn[value::BYTEARRAY()] = "we::type::bytearray";
          tn[value::LIST()] = "std::list<pnet::type::value::value_type>";
          tn[value::SET()] = "std::set<pnet::type::value::value_type>";
          tn[value::MAP()] = "std::map<pnet::type::value::value_type,pnet::type::value::value_type>";

          return tn;
        }

        const std::string typename_complete (std::string const& tname)
        {
          static map_type tn (init_typenames_complete());

          const map_type::const_iterator pos (tn.find (tname));

          if (pos == tn.end())
          {
            return (tname + "::" + tname);
          }

          return pos->second;
        }
      }

      complete::complete (std::string const& tname)
        : _tname (tname)
      {}
      std::string const& complete::tname() const
      {
        return _tname;
      }
      std::ostream& operator<< (std::ostream& os, complete const& c)
      {
        return os << typename_complete (c.tname());
      }
    }
  }
}
