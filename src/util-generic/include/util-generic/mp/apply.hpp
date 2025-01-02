// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace fhg
{
  namespace util
  {
    namespace mp
    {
      namespace detail
      {
        template<template<typename> class, typename> struct apply;
      }

      //! Apply an `Operation<typename> op` to all elements of
      //! type-sequence `Sequence = sequence_type<Types...>` and
      //! return `sequence_type<op (Types)...>`.
      template<template<typename> class Operation, typename Sequence>
        using apply = typename detail::apply<Operation, Sequence>::type;
    }
  }
}

#include <util-generic/mp/apply.ipp>
