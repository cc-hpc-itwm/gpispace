// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <we/expr/eval/context.hpp>
#include <we/loader/api-guard.hpp>

#include <drts/worker/context_fwd.hpp>

#include <map>
#include <string>

namespace we
{
  namespace loader
  {
    using WrapperFunction = void (*)( drts::worker::context*
                                    , const expr::eval::context&
                                    , expr::eval::context&
                                    , const std::map<std::string, void *>&
                                    );

    class GSPC_DLLEXPORT IModule
    {
    public:
      virtual ~IModule() = default;

      virtual void add_function (std::string const&, WrapperFunction) = 0;

      IModule() = default;
      IModule (IModule const&) = delete;
      IModule& operator= (IModule const&) = delete;
      IModule (IModule&&) = delete;
      IModule& operator= (IModule&&) = delete;
    };
  }
}
