// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace fhg
{
  namespace util
  {
    namespace mp
    {
      namespace detail
      {
        template<typename> struct remove_duplicates;
      }

      //! Given a type-sequence Sequence = sequence_types<Types...>,
      //! returns sequence_type<unique (Types...)>.
      //! \note Is O(Types^2), so use with care.
      template<typename Sequence>
        using remove_duplicates = typename detail::remove_duplicates<Sequence>::type;
    }
  }
}

#include <util-generic/mp/remove_duplicates.ipp>
