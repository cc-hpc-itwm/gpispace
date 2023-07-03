// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace fhg
{
  namespace util
  {
   namespace serialization
   {
      namespace detail
      {
        template<typename T> struct serialize_by_member;
      }
    }
  }
}

#define FHG_UTIL_SERIALIZATION_ACCESS_IMPL(type_)                       \
  friend ::fhg::util::serialization::detail::serialize_by_member<type_>
