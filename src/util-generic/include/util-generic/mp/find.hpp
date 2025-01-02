// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>

namespace fhg
{
  namespace util
  {
    namespace mp
    {
      namespace detail
      {
        template <typename, std::size_t, typename...> struct find;
      }

      //! Returns an std::integral_constant<std::size_t> with the
      //! index of type T in types Types.
      //! \note un-instantiated if Types does not contain T
      //! \note returns the first position of T if Types contains T
      //! multiple times.
      template <typename T, typename... Types>
        using find = detail::find<T, 0, Types...>;
    }
  }
}

#include <util-generic/mp/find.ipp>
