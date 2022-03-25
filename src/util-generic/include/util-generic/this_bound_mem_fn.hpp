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

#include <functional>
#include <utility>

namespace fhg
{
  namespace util
  {
    namespace
    {
      template<typename This, typename Memfn>
        struct this_bound_mem_fn
      {
        This* _this;
        Memfn _fun;

        template<typename... Args>
          typename Memfn::result_type operator() (Args&&... args) const
        {
          return _fun (_this, std::forward<Args> (args)...);
        }
      };
    }

    template<typename This, typename Fun>
      auto bind_this (This* that, Fun&& fun)
        -> this_bound_mem_fn
             <This, decltype (std::mem_fn (std::forward<Fun> (fun)))>
    {
      return {that, std::mem_fn (std::forward<Fun> (fun))};
    }
  }
}
