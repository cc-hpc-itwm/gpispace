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

#include <util-generic/callable_signature.hpp>

#include <iterator>
#include <utility>
#include <type_traits>

namespace fhg
{
  namespace util
  {
    //! Apply \a fun asynchronously (using \c std::async) to all
    //! elements between \a begin and \a end, and wait for the results.
    //! \see wait_and_collect_exceptions()
    template< typename Iterator
            , typename Fun
            , typename = typename std::enable_if
                 <is_callable<Fun, void (decltype (*std::declval<Iterator>()))>{}>::type
            >
      void asynchronous (Iterator begin, Iterator end, Fun&& fun);

    //! Apply \a fun asynchronously (using \c std::async) to all
    //! elements in \a collection, and wait for the results.
    //! \see wait_and_collect_exceptions()
    template< typename Collection
            , typename Fun
            , typename = typename std::enable_if
                <is_callable<Fun, void (decltype (*std::begin (std::declval<Collection>())))>{}>::type
            >
      void asynchronous (Collection collection, Fun&& fun);
  }
}

#include <util-generic/asynchronous.ipp>
