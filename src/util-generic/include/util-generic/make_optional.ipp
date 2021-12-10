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

#include <boost/optional.hpp>

namespace fhg
{
  namespace util
  {
#define FHG_UTIL_MAKE_OPTIONAL_IMPL(cond_, how_...)                  \
    fhg::util::detail::make_optional (cond_, [&] { return how_; })

    namespace detail
    {
      template<typename Fun>
        auto make_optional (bool cond, Fun&& fun)
          -> ::boost::optional<decltype (fun())>
      {
        using T = ::boost::optional<decltype (fun())>;
        return cond ? T (fun()) : T();
      }
    }
  }
}
