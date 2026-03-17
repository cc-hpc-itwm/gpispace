// Copyright (C) 2013-2014,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/signature/complete.hpp>

#include <gspc/we/type/shared.hpp>
#include <gspc/we/type/value/name.hpp>

#include <iostream>
#include <unordered_map>



    namespace gspc::pnet::type::signature
    {
      namespace
      {
        using map_type = std::unordered_map<std::string, std::string>;

        map_type init_typenames_complete()
        {
          map_type tn;

          tn[value::CONTROL()] = "gspc::we::type::literal::control";
          tn[value::BOOL()] = "bool";
          tn[value::INT()] = "int";
          tn[value::LONG()] = "long";
          tn[value::UINT()] = "unsigned int";
          tn[value::ULONG()] = "unsigned long";
          tn[value::FLOAT()] = "float";
          tn[value::DOUBLE()] = "double";
          tn[value::CHAR()] = "char";
          tn[value::STRING()] = "std::string";
          tn[value::BITSET()] = "gspc::pnet::type::bitsetofint::type";
          tn[value::BYTEARRAY()] = "gspc::we::type::bytearray";
          tn[value::SHARED()] = "gspc::we::type::shared";
          tn[value::LIST()] = "std::list<gspc::pnet::type::value::value_type>";
          tn[value::SET()] = "std::set<gspc::pnet::type::value::value_type>";
          tn[value::MAP()] = "std::map<gspc::pnet::type::value::value_type,gspc::pnet::type::value::value_type>";

          return tn;
        }

        std::string typename_complete (std::string const& tname)
        {
          static map_type tn (init_typenames_complete());

          const map_type::const_iterator pos (tn.find (tname));

          if (pos == tn.end())
          {
            // Handle shared_PLACENAME types
            //
            if (we::type::shared::cleanup_place (tname))
            {
              return "gspc::we::type::shared";
            }

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
