// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
        using map_type = std::unordered_map<std::string, std::string>;

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

        std::string typename_complete (std::string const& tname)
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
