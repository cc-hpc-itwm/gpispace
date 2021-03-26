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

#include <boost/optional.hpp>
#include <boost/variant.hpp>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      using namespace ::boost;

      namespace
      {
        template<typename T> class get_or_none_impl
          : public boost::static_visitor<boost::optional<T&>>
        {
        public:
          boost::optional<T&> operator() (T& val) const
          {
            return val;
          }

          template<typename U> boost::optional<T&> operator() (const U&) const
          {
            return boost::none;
          }
        };

        template<typename T> class is_of_type_impl
          : public boost::static_visitor<bool>
        {
        public:
          bool operator() (const T&) const
          {
            return true;
          }

          template<typename U> bool operator() (const U&) const
          {
            return false;
          }
        };
      }

      template<typename T, typename variant_type>
        boost::optional<const T&> get_or_none (const variant_type& variant)
      {
        return boost::apply_visitor (get_or_none_impl<const T>(), variant);
      }

      template<typename T, typename variant_type>
        boost::optional<T&> get_or_none (variant_type& variant)
      {
        return boost::apply_visitor (get_or_none_impl<T>(), variant);
      }

      template<typename T, typename variant_type>
        bool is_of_type (const variant_type& variant)
      {
        return boost::apply_visitor (is_of_type_impl<T>(), variant);
      }
    }
  }
}
