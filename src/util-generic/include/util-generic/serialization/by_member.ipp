// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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
