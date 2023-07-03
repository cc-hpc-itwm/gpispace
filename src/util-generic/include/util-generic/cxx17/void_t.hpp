// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//! \note n3911: TransformationTrait Alias `void_t`

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      template<typename...> struct make_void { using type = void; };
      template<typename... Ts> using void_t
        = typename make_void<Ts...>::type;
    }
  }
}
