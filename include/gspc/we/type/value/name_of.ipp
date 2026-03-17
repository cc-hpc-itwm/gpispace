// Copyright (C) 2013,2018,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/value/name.hpp>
#include <gspc/we/type/value/name_of.hpp>

#include <gspc/we/type/value.hpp>



    namespace gspc::pnet::type::value
    {
#define NAME_OF(_name, _type...)                                         \
      template<> inline std::string const& name_of<_type> (_type const&) \
      {                                                                  \
        return _name();                                                  \
      }

      NAME_OF (CONTROL, we::type::literal::control)
      NAME_OF (BOOL, bool)
      NAME_OF (INT, int)
      NAME_OF (LONG, long)
      NAME_OF (UINT, unsigned int)
      NAME_OF (ULONG, unsigned long)
      NAME_OF (FLOAT, float)
      NAME_OF (DOUBLE, double)
      NAME_OF (CHAR, char)
      NAME_OF (STRING, std::string)
      NAME_OF (BITSET, pnet::type::bitsetofint::type)
      NAME_OF (BYTEARRAY, we::type::bytearray)
      NAME_OF (BIGINT, bigint_type)
      NAME_OF (SHARED, we::type::shared)
      NAME_OF (LIST, std::list<value_type>)
      NAME_OF (SET, std::set<value_type>)
      NAME_OF (MAP, std::map<value_type,value_type>)
      NAME_OF (STRUCT, structured_type)

#undef NAME_OF
    }
