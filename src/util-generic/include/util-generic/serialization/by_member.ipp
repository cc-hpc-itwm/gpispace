// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/serialization/base_object.hpp>

namespace fhg
{
  namespace util
  {
   namespace serialization
   {
      namespace detail
      {
        template<typename T>
          struct serialize_by_member
        {
          template<typename A>
            void append (A&, T&);
          template<typename A, typename B, typename... R>
            void append (A&, T&, detail::base_class_t<B>, R...);
          template<typename A, typename Type, typename... R>
            void append (A&, T& x, Type T::*, R...);

          template<typename Archive>
            void operator() (Archive&, T&);
        };

        template<typename T>
          template<typename Archive>
            void serialize_by_member<T>::append (Archive&, T&)
        {
        }

        template<typename T>
          template<typename Archive, typename Type, typename... Rest>
            void serialize_by_member<T>::append
              (Archive& ar, T& x, detail::base_class_t<Type>, Rest... rest)
        {
          ar & ::boost::serialization::base_object<Type> (x);
          append (ar, x, rest...);
        }

        template<typename T>
          template<typename Archive, typename Type, typename... Rest>
            void serialize_by_member<T>::append
              (Archive& ar, T& x, Type T::* member, Rest... rest)
        {
          ar & x.*member;
          append (ar, x, rest...);
        }
      }
    }
  }
}

#define FHG_UTIL_SERIALIZATION_DETAIL_BY_MEMBER(type_, base_or_members_...)    \
namespace fhg                                                                  \
{                                                                              \
  namespace util                                                               \
  {                                                                            \
    namespace serialization                                                    \
    {                                                                          \
      namespace detail                                                         \
      {                                                                        \
        template<>                                                             \
          template<typename Archive>                                           \
            void serialize_by_member<type_>::operator() (Archive& ar, type_& x)\
        {                                                                      \
          append (ar, x, base_or_members_);                                    \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }                                                                            \
}                                                                              \
namespace boost                                                                \
{                                                                              \
  namespace serialization                                                      \
  {                                                                            \
    template<typename Archive>                                                 \
      void serialize (Archive& ar, type_& x, unsigned int)                     \
    {                                                                          \
      fhg::util::serialization::detail::serialize_by_member<type_>{} (ar, x);  \
    }                                                                          \
  }                                                                            \
}
