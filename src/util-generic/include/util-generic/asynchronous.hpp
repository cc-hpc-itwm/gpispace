// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
