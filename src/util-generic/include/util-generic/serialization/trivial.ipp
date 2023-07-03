// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/warning.hpp>

#include <boost/serialization/split_free.hpp>

namespace fhg
{
  namespace util
  {
    namespace serialization
    {
      namespace detail
      {
        template<typename T, typename For>
          struct require_type_trivially_serializable
        {
          static_assert ( is_trivially_serializable<T>{}
                        , "type shall be trivially serializable"
                        );
        };
        template<typename T, typename For>
          struct require_type_is_derived_of
        {
          //! \note no "directly derived" possible in C++, sadly.
          static_assert (std::is_base_of<T, For>{}, "type shall be derived of");
        };

        template<typename For, typename... Types>
          struct require_types_trivially_serializable : std::true_type {};
        template<typename For, typename Type, typename... Rest>
          struct require_types_trivially_serializable<For, Type For::*, Rest...>
            : require_types_trivially_serializable<For, Rest...>
        {
          //! \note Does not use inheritance for require_s to avoid
          //! -Wunreachable-base: When using macro with members of the
          //! same type, it would be possible to generate inheriting
          //! in a diamond shape. While this never is an issue since
          //! we don't even access the base class anywhere, -Werror
          //! makes it break compilation. Instead of inheriting for
          //! the check, only inherit for recursion and have checks as
          //! members. This breaks the diamond without virtual
          //! inheritance, which wouldn't work since virtually
          //! inheriting classes are not usable in constexpr contexts.
          static constexpr require_type_trivially_serializable<Type, For> _{};
        };
        template<typename For, typename Type, typename... Rest>
          struct require_types_trivially_serializable<For, base_class_t<Type>, Rest...>
            : require_types_trivially_serializable<For, Rest...>
        {
          static constexpr require_type_trivially_serializable<Type, For> _1{};
          static constexpr require_type_is_derived_of<Type, For> _2{};
        };

        template<typename For, typename... MemberTypes>
          constexpr bool require_member_types_are_trivially_serializable
            (MemberTypes...)
        {
          return require_types_trivially_serializable<For, MemberTypes...>{};
        }

#define FHG_UTIL_SERIALIZATION_DETAIL_TRIVIAL(type_, members_...)              \
static_assert                                                                  \
  ( fhg::util::serialization::detail::require_member_types_are_trivially_serializable<type_> \
     (members_)                                                                \
  , "not all members are trivially serializable"                               \
  );                                                                           \
BOOST_IS_BITWISE_SERIALIZABLE (type_)                                          \
BOOST_SERIALIZATION_SPLIT_FREE (type_)                                         \
namespace boost                                                                \
{                                                                              \
  namespace serialization                                                      \
  {                                                                            \
    template<typename Archive>                                                 \
      inline void load (Archive& ar, type_& x, unsigned int)                   \
    {                                                                          \
      /* workaround for missing explicit instantiation declaration in text_iarchive */ \
      DISABLE_WARNING_CLANG_UNDEFINED_FUNC_TEMPLATE()                          \
      ar.load_binary (&x, sizeof (x));                                         \
      RESTORE_WARNING_CLANG_UNDEFINED_FUNC_TEMPLATE()                          \
    }                                                                          \
    template<typename Archive>                                                 \
      inline void save (Archive& ar, type_ const& x, unsigned int)             \
    {                                                                          \
      /* workaround for missing explicit instantiation declaration in text_oarchive */ \
      DISABLE_WARNING_CLANG_UNDEFINED_FUNC_TEMPLATE()                          \
      ar.save_binary (&x, sizeof (x));                                         \
      RESTORE_WARNING_CLANG_UNDEFINED_FUNC_TEMPLATE()                          \
    }                                                                          \
    template<typename Archive>                                                 \
      inline void load_construct_data (Archive&, type_*, unsigned int)         \
    {                                                                          \
    }                                                                          \
    template<typename Archive>                                                 \
      inline void save_construct_data (Archive&, type_ const*, unsigned int)   \
    {                                                                          \
    }                                                                          \
  }                                                                            \
}
      }
    }
  }
}
