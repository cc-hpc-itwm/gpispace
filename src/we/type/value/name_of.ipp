// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/value/name.hpp>
#include <we/type/value/name_of.hpp>

#include <we/type/value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
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
      NAME_OF (BITSET, bitsetofint::type)
      NAME_OF (BYTEARRAY, we::type::bytearray)
      NAME_OF (LIST, std::list<value_type>)
      NAME_OF (SET, std::set<value_type>)
      NAME_OF (MAP, std::map<value_type,value_type>)
      NAME_OF (STRUCT, structured_type)

#undef NAME_OF
    }
  }
}
