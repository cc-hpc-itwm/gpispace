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
