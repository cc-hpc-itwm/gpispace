// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <exception>
#include <utility>

namespace fhg
{
  namespace util
  {
    template<typename Exception, typename... ExceptionArgs, typename Fun>
      [[deprecated ("manually use throw_with_nested")]]
      auto nest_exceptions (Fun&& fun, ExceptionArgs&&... exception_args)
      -> decltype (fun())
    {
      try
      {
        return fun();
      }
      catch (...)
      {
        std::throw_with_nested
          (Exception (std::forward<ExceptionArgs> (exception_args)...));
      }
    }
  }
}
