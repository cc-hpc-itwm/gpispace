// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <iosfwd>
#include <set>
#include <string>
#include <vector>

namespace expr
{
  //! Types of expressions are types of `pnet::type::value::value_types`s

  namespace type
  {
    struct Control{};
    struct Boolean{};
    struct Int{};
    struct Long{};
    struct UInt{};
    struct ULong{};
    struct Float{};
    struct Double{};
    struct Char{};
    struct String{};
    struct Bitset{};
    struct Bytearray{};

    template<typename Type> struct ListT
    {
      ListT (Type element);

      Type _element;
    };

    template<typename Type> struct SetT
    {
      SetT (Type element);

      Type _element;
    };

    template<typename Type> struct MapT
    {
      MapT (Type key, Type value);

      Type _key;
      Type _value;
    };

    template<typename Type> struct StructT
    {
      struct Field
      {
        using Name = std::string;

        Field (Name name, Type type);

        Name _name;
        Type _type;
      };

      using Fields = std::vector<Field>;

      StructT (Fields fields);

      Fields _fields;
    };

    // Multiple and arbitrary types, e.g.
    //   List(1,"") has type List [{int, string}]
    // and
    //   map_get_assignment (Map[], 1) has type Any == Types{}
    template<typename Type> struct TypesT
    {
      using Types = std::set<Type>;

      TypesT();
      TypesT (Type type);
      TypesT (Types types);

      bool is_any() const;                     // no type
      ::boost::optional<Type> singleton() const; // exactly one type
      bool is_multi() const;                   // more than one type

      Types _types;
    };

    //! \todo remove we::signature, use this type instead
    //! \todo parse homogeneous types, e.g. 'list|int|'
    using TypeV = ::boost::make_recursive_variant
        < Control
        , Boolean
        , Int
        , Long
        , UInt
        , ULong
        , Float
        , Double
        , Char
        , String
        , Bitset
        , Bytearray
        , ListT<::boost::recursive_variant_>
        , SetT<::boost::recursive_variant_>
        , MapT<::boost::recursive_variant_>
        , StructT<::boost::recursive_variant_>
        , TypesT<::boost::recursive_variant_>
        >::type;
  }
}

namespace expr
{
  using Type = type::TypeV;

  namespace type
  {
    using List = ListT<Type>;
    using Set = SetT<Type>;
    using Map = MapT<Type>;
    using Struct = StructT<Type>;
    using Types = TypesT<Type>;
  }
}

namespace expr
{
  namespace type
  {
    std::ostream& operator<< (std::ostream&, Control const&);
    std::ostream& operator<< (std::ostream&, Boolean const&);
    std::ostream& operator<< (std::ostream&, Int const&);
    std::ostream& operator<< (std::ostream&, Long const&);
    std::ostream& operator<< (std::ostream&, UInt const&);
    std::ostream& operator<< (std::ostream&, ULong const&);
    std::ostream& operator<< (std::ostream&, Float const&);
    std::ostream& operator<< (std::ostream&, Double const&);
    std::ostream& operator<< (std::ostream&, Char const&);
    std::ostream& operator<< (std::ostream&, String const&);
    std::ostream& operator<< (std::ostream&, Bitset const&);
    std::ostream& operator<< (std::ostream&, Bytearray const&);
    std::ostream& operator<< (std::ostream&, List const&);
    std::ostream& operator<< (std::ostream&, Set const&);
    std::ostream& operator<< (std::ostream&, Map const&);
    std::ostream& operator<< (std::ostream&, Struct const&);
    std::ostream& operator<< (std::ostream&, Types const&);
  }
}

namespace expr
{
  namespace type
  {
    bool operator== (Control const&, Control const&);
    bool operator== (Boolean const&, Boolean const&);
    bool operator== (Int const&, Int const&);
    bool operator== (Long const&, Long const&);
    bool operator== (UInt const&, UInt const&);
    bool operator== (ULong const&, ULong const&);
    bool operator== (Float const&, Float const&);
    bool operator== (Double const&, Double const&);
    bool operator== (Char const&, Char const&);
    bool operator== (String const&, String const&);
    bool operator== (Bitset const&, Bitset const&);
    bool operator== (Bytearray const&, Bytearray const&);
    bool operator== (List const&, List const&);
    bool operator== (Set const&, Set const&);
    bool operator== (Map const&, Map const&);
    bool operator== (Struct::Field const&, Struct::Field const&);
    bool operator== (Struct const&, Struct const&);
    bool operator== (Types const&, Types const&);
  }
}

namespace expr
{
  namespace type
  {
    bool operator!= (Control const&, Control const&);
    bool operator!= (Boolean const&, Boolean const&);
    bool operator!= (Int const&, Int const&);
    bool operator!= (Long const&, Long const&);
    bool operator!= (UInt const&, UInt const&);
    bool operator!= (ULong const&, ULong const&);
    bool operator!= (Float const&, Float const&);
    bool operator!= (Double const&, Double const&);
    bool operator!= (Char const&, Char const&);
    bool operator!= (String const&, String const&);
    bool operator!= (Bitset const&, Bitset const&);
    bool operator!= (Bytearray const&, Bytearray const&);
    bool operator!= (List const&, List const&);
    bool operator!= (Set const&, Set const&);
    bool operator!= (Map const&, Map const&);
    bool operator!= (Struct::Field const&, Struct::Field const&);
    bool operator!= (Struct const&, Struct const&);
    bool operator!= (Types const&, Types const&);
  }
}

namespace expr
{
  namespace type
  {
    bool operator< (Control const&, Control const&);
    bool operator< (Boolean const&, Boolean const&);
    bool operator< (Int const&, Int const&);
    bool operator< (Long const&, Long const&);
    bool operator< (UInt const&, UInt const&);
    bool operator< (ULong const&, ULong const&);
    bool operator< (Float const&, Float const&);
    bool operator< (Double const&, Double const&);
    bool operator< (Char const&, Char const&);
    bool operator< (String const&, String const&);
    bool operator< (Bitset const&, Bitset const&);
    bool operator< (Bytearray const&, Bytearray const&);
    bool operator< (List const&, List const&);
    bool operator< (Set const&, Set const&);
    bool operator< (Map const&, Map const&);
    bool operator< (Struct::Field const&, Struct::Field const&);
    bool operator< (Struct const&, Struct const&);
    bool operator< (Types const&, Types const&);
  }
}

namespace expr
{
  namespace type
  {
    template<> ListT<Type>::ListT (Type element);
    template<> SetT<Type>::SetT (Type element);
    template<> MapT<Type>::MapT (Type key, Type value);
    template<> StructT<Type>::Field::Field (Name name, Type type);
    template<> StructT<Type>::StructT (Fields fields);
    template<> TypesT<Type>::TypesT();
    template<> TypesT<Type>::TypesT (Type type);
    template<> TypesT<Type>::TypesT (Types types);
    template<> bool TypesT<Type>::is_any() const;
    template<> ::boost::optional<Type> TypesT<Type>::singleton() const;
    template<> bool TypesT<Type>::is_multi() const;
  }
}

namespace expr
{
  namespace type
  {
    Type Any();

    bool is_any (Type const& type);
    bool is_singleton (Type const& type);
    bool is_multi (Type const& type);
  }
}
