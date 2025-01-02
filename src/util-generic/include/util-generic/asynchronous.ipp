// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
