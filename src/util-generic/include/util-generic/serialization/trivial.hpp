// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/cxx17/logical_operator_type_traits.hpp>
#include <util-generic/serialization/detail/base_class.hpp>

#include <boost/serialization/is_bitwise_serializable.hpp>

#include <type_traits>

namespace fhg
{
  namespace util
  {
    namespace serialization
    {
      //! Check if all given Types are trivially serializable.
      template<typename... Types>
        using is_trivially_serializable
          = cxx17::conjunction
              <::boost::serialization::is_bitwise_serializable<Types>...>;

      //! Let the serialization of type_ with the
      //! base_or_members_... be trivial. base_or_members_... are
      //! asserted to be trivially serializable.
      //! \see base_class<T>()
#define FHG_UTIL_SERIALIZATION_TRIVIAL(type_, base_or_members_...)      \
      FHG_UTIL_SERIALIZATION_DETAIL_TRIVIAL (type_, base_or_members_)
    }
  }
}

#include <util-generic/serialization/trivial.ipp>
