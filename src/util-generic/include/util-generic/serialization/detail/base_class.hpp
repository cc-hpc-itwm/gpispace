// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace fhg
{
  namespace util
  {
    namespace serialization
    {
      namespace detail
      {
        template<typename T>
          struct base_class_t {};
      }

      //! Tag type for marking a base class in serialization generators.
      template<typename T>
        constexpr static detail::base_class_t<T> base_class() { return {}; }
    }
  }
}
