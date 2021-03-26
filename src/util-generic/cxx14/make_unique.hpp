// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <memory>

#if HAS_STD_MAKE_UNIQUE

namespace fhg
{
  namespace util
  {
    namespace cxx14
    {
      using std::make_unique;
    }
  }
}

#else

#include <cstddef>
#include <type_traits>
#include <utility>

//! \note n3656: make_unique

namespace fhg
{
  namespace util
  {
    namespace cxx14
    {
      namespace detail
      {
        template<typename T> struct make_unique_switch
        {
          using single_object = std::unique_ptr<T>;
        };
        template<typename T> struct make_unique_switch<T[]>
        {
          using unknown_bound = std::unique_ptr<T[]>;
        };
        template<typename T, size_t N> struct make_unique_switch<T[N]>
        {
          using known_bound = void;
        };
      }

      template<typename T, typename... Args>
        typename detail::make_unique_switch<T>::single_object
          make_unique (Args&&... args)
      {
        return std::unique_ptr<T> (new T (std::forward<Args> (args)...));
      }

      template<typename T>
        typename detail::make_unique_switch<T>::unknown_bound
          make_unique (std::size_t count)
      {
        using U = typename std::remove_extent<T>::type;
        return std::unique_ptr<T> (new U[count]());
      }

      template<typename T, typename... Args>
        typename detail::make_unique_switch<T>::known_bound
          make_unique (Args&&...) = delete;
    }
  }
}

#endif
