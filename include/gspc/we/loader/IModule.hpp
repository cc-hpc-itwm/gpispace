// Copyright (C) 2010,2012-2015,2021-2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/we/expr/eval/context.hpp>
#include <gspc/we/loader/api-guard.hpp>

#include <gspc/drts/worker/context_fwd.hpp>

#include <map>
#include <string>


  namespace gspc::we::loader
  {
    using WrapperFunction = void (*)( drts::worker::context*
                                    , const we::expr::eval::context&
                                    , we::expr::eval::context&
                                    , const std::map<std::string, void *>&
                                    );

    class GSPC_EXPORT IModule
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
