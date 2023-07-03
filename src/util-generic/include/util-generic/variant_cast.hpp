// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdexcept>

namespace fhg
{
  namespace util
  {
    struct bad_variant_cast : std::logic_error
    {
      template<typename From, typename To, typename Element>
        static bad_variant_cast make();

    private:
      bad_variant_cast (std::string, std::string, std::string);
    };

    //! Cast a From=::boost::variant<T...> to To=::boost::variant<U...>
    //! with runtime check to throw on cases where a T is not in U.
    template<typename To, typename From>
      To variant_cast (From&&);

    namespace detail
    {
      template<typename From, typename To> struct variants_cast_return_type;
    }

    //! Call variant_cast<To> on all elements of
    //! From=Sequence<::boost::variant<T...>>, returning Sequence<To>.
    template<typename To, typename From>
      typename detail::variants_cast_return_type<From, To>::type
        variants_cast (From&&);
  }
}

#include <util-generic/variant_cast.ipp>
