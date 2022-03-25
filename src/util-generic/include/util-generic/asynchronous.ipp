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

#include <util-generic/wait_and_collect_exceptions.hpp>

#include <functional>
#include <future>
#include <vector>

namespace fhg
{
  namespace util
  {
    template< typename Iterator
            , typename Fun
            , typename
            >
      void asynchronous (Iterator begin, Iterator end, Fun&& fun)
    {
      std::vector<std::future<void>> executions;

      while (begin != end)
      {
        executions.emplace_back
          (std::async (std::launch::async, std::bind (fun, *begin)));

        ++begin;
      }

      wait_and_collect_exceptions (executions);
    }

    //! \todo cxx14: use cbegin, cend
    template< typename Collection
            , typename Fun
            , typename
            >
      void asynchronous (Collection collection, Fun&& fun)
    {
      return asynchronous<decltype (std::begin (collection))>
        ( std::begin (collection), std::end (collection)
        , std::forward<Fun> (fun)
        );
    }
  }
}
