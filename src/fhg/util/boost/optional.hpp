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

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      using namespace ::boost;
      template<typename T, typename U>
      boost::optional<U> fmap (U (*f)(T const&), boost::optional<T> const& m)
      {
        if (m)
        {
          return f (*m);
        }
        else
        {
          return boost::none;
        }
      }

      template<typename Exception, typename T, typename... Args>
        T get_or_throw (boost::optional<T> const& optional, Args&&... args)
      {
        if (!optional)
        {
          throw Exception (args...);
        }

        return optional.get();
      }
    }
  }
}

namespace boost
{
  template<typename T>
  std::ostream& operator<< (std::ostream& s, boost::optional<T> const& x)
  {
    return x ? (s << "Just " << *x) : (s << "Nothing");
  }
}
