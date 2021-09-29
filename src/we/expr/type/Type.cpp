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

#include <we/expr/type/Type.hpp>

#include <we/type/value/name.hpp>

#include <util-generic/functor_visitor.hpp>
#include <util-generic/print_container.hpp>

#include <ostream>
#include <tuple>

namespace expr
{
  namespace type
  {
#define IMPLEMENT_OPERATOR_STREAM(T, x, os)                     \
    std::ostream& operator<< (std::ostream& os, T const& x)

#define IMPLEMENT_OPERATOR_STREAM_FALLBACK(T,S)                 \
    IMPLEMENT_OPERATOR_STREAM(T,,os)                            \
    {                                                           \
      return os << ::pnet::type::value::S();                    \
    }

    IMPLEMENT_OPERATOR_STREAM_FALLBACK (Control, CONTROL)
    IMPLEMENT_OPERATOR_STREAM_FALLBACK (Boolean, BOOL)
    IMPLEMENT_OPERATOR_STREAM_FALLBACK (Int, INT)
    IMPLEMENT_OPERATOR_STREAM_FALLBACK (Long, LONG)
    IMPLEMENT_OPERATOR_STREAM_FALLBACK (UInt, UINT)
    IMPLEMENT_OPERATOR_STREAM_FALLBACK (ULong, ULONG)
    IMPLEMENT_OPERATOR_STREAM_FALLBACK (Float, FLOAT)
    IMPLEMENT_OPERATOR_STREAM_FALLBACK (Double, DOUBLE)
    IMPLEMENT_OPERATOR_STREAM_FALLBACK (Char, CHAR)
    IMPLEMENT_OPERATOR_STREAM_FALLBACK (String, STRING)
    IMPLEMENT_OPERATOR_STREAM_FALLBACK (Bitset, BITSET)
    IMPLEMENT_OPERATOR_STREAM_FALLBACK (Bytearray, BYTEARRAY)

#undef IMPLEMENT_OPERATOR_STREAM_FALLBACK

    IMPLEMENT_OPERATOR_STREAM (List, list, os)
    {
      return os << "List [" << list._element << "]";
    }

    IMPLEMENT_OPERATOR_STREAM (Set, set, os)
    {
      return os << "Set [" << set._element << "]";
    }

    IMPLEMENT_OPERATOR_STREAM (Map, map, os)
    {
      return os << "Map [" << map._key << " -> " << map._value << "]";
    }

    IMPLEMENT_OPERATOR_STREAM (Struct, st, os)
    {
      return os
        << "Struct "
        << fhg::util::print_container
           ( "[", ", ", "]", st._fields
           , [] (auto& s, auto const& field) -> decltype (s)
             {
               return s << field._name << " :: " << field._type;
             }
           )
        ;
    }

    IMPLEMENT_OPERATOR_STREAM (Types, types, os)
    {
      if (types.is_any())
      {
        return os << "Any";
      }

      if (auto type = types.singleton())
      {
        return os << *type;
      }

      return os << fhg::util::print_container ("{", ", ", "}", types._types);
    }

#undef IMPLEMENT_OPERATOR_STREAM
  }
}

namespace expr
{
  namespace type
  {
#define IMPLEMENT_OPERATOR_EQUAL(T, lhs, rhs)           \
    bool operator!= (T const& _lhs, T const& _rhs)      \
    {                                                   \
      return !(_lhs == _rhs);                           \
    }                                                   \
    bool operator== (T const& lhs, T const& rhs)

#define IMPLEMENT_OPERATOR_LESS(T, lhs, rhs)    \
    bool operator< (T const& lhs, T const& rhs)

#define IMPLEMENT_LITERAL_OPERATORS(T)          \
    IMPLEMENT_OPERATOR_EQUAL(T,,)               \
    {                                           \
      return true;                              \
    }                                           \
    IMPLEMENT_OPERATOR_LESS(T,,)                \
    {                                           \
      return false;                             \
    }

    IMPLEMENT_LITERAL_OPERATORS (Control)
    IMPLEMENT_LITERAL_OPERATORS (Boolean)
    IMPLEMENT_LITERAL_OPERATORS (Int)
    IMPLEMENT_LITERAL_OPERATORS (Long)
    IMPLEMENT_LITERAL_OPERATORS (UInt)
    IMPLEMENT_LITERAL_OPERATORS (ULong)
    IMPLEMENT_LITERAL_OPERATORS (Float)
    IMPLEMENT_LITERAL_OPERATORS (Double)
    IMPLEMENT_LITERAL_OPERATORS (Char)
    IMPLEMENT_LITERAL_OPERATORS (String)
    IMPLEMENT_LITERAL_OPERATORS (Bitset)
    IMPLEMENT_LITERAL_OPERATORS (Bytearray)

#undef IMPLEMENT_LITERAL_OPERATORS

#define IMPLEMENT_OPERATORS(T)                  \
    IMPLEMENT_OPERATOR_EQUAL (T, lhs, rhs)      \
    {                                           \
      return ESSENCE (lhs) == ESSENCE (rhs);    \
    }                                           \
    IMPLEMENT_OPERATOR_LESS (T, lhs, rhs)       \
    {                                           \
      return ESSENCE (lhs) < ESSENCE (rhs);     \
    }

#define ESSENCE(x) std::tie (x._element)
    IMPLEMENT_OPERATORS (List)
#undef ESSENCE

#define ESSENCE(x) std::tie (x._element)
    IMPLEMENT_OPERATORS (Set)
#undef ESSENCE

#define ESSENCE(x) std::tie (x._key, x._value)
    IMPLEMENT_OPERATORS (Map)
#undef ESSENCE

#define ESSENCE(x) std::tie (x._name, x._type)
    IMPLEMENT_OPERATORS (Struct::Field)
#undef ESSENCE

#define ESSENCE(x) std::tie (x._fields)
    IMPLEMENT_OPERATORS (Struct)
#undef ESSENCE

#define ESSENCE(x) std::tie (x._types)
    IMPLEMENT_OPERATORS (Types)
#undef ESSENCE

#undef IMPLEMENT_OPERATORS
#undef IMPLEMENT_OPERATOR_LESS
#undef IMPLEMENT_OPERATOR_EQUAL
  }
}

namespace expr
{
  namespace type
  {
    template<> ListT<Type>::ListT (Type element)
      : _element (element)
    {}

    template<> SetT<Type>::SetT (Type element)
      : _element (element)
    {}

    template<> MapT<Type>::MapT (Type key, Type value)
      : _key (key)
      , _value (value)
    {}

    template<> StructT<Type>::Field::Field (Name name, Type type)
      : _name (name)
      , _type (type)
    {}

    template<> StructT<Type>::StructT (Fields fields)
      : _fields (fields)
    {}

    template<> TypesT<Type>::TypesT()
      : TypesT<Type> (TypesT<Type>::Types {})
    {}
    template<> TypesT<Type>::TypesT (Type type)
      : TypesT (TypesT<Type>::Types {type})
    {}
    template<> TypesT<Type>::TypesT (Types types)
      : _types (types)
    {}

    template<> bool TypesT<Type>::is_any() const
    {
      return _types.empty();
    }
    template<> boost::optional<Type> TypesT<Type>::singleton() const
    {
      if (_types.size() == 1)
      {
        return *_types.begin();
      }

      return {};
    }
    template<> bool TypesT<Type>::is_multi() const
    {
      return _types.size() > 1;
    }
  }
}

namespace expr
{
  namespace type
  {
    Type Any()
    {
      return Types{};
    }

    bool is_any (Type const& type)
    {
      return fhg::util::visit<bool>
        ( type
        , [] (Types const& types)
          {
            return types.is_any();
          }
        , [] (auto const&)
          {
            return false;
          }
        );
    }
    bool is_singleton (Type const& type)
    {
      return fhg::util::visit<bool>
        ( type
        , [] (Types const& types)
          {
            return !!types.singleton();
          }
        , [] (auto const&)
          {
            return true;
          }
        );
    }
    bool is_multi (Type const& type)
    {
      return fhg::util::visit<bool>
        ( type
        , [] (Types const& types)
          {
            return types.is_multi();
          }
        , [] (auto const&)
          {
            return false;
          }
        );
    }
  }
}
