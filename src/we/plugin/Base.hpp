// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <we/expr/eval/context.hpp>
#include <we/type/value.hpp>

#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      using Context = ::expr::eval::context;
      using PlaceName = std::string;
      using Value = ::pnet::type::value::value_type;

      //! It is safe to use a temporary for the place name.
      //! Calls to put_token will not block, however, the execution
      //! will happen asynchronously.
      using PutToken = std::function<void (PlaceName const&, Value)>;

      //! All methods including constructor and destructor shall not
      //! block.
      struct GSPC_DLLEXPORT Base
      {
        //! executed after expression has been evaluated
        //! The callback is guaranteed to be valid during object life
        //! time.
        //! Plugin::Plugin (Context const&, PutToken);

        //! executed after expression input has been bound
        virtual void before_eval (Context const&) = 0;

        //! executed after expression has been evaluated
        virtual void after_eval (Context const&) = 0;

        virtual ~Base() = default;

        Base() = default;
        Base (Base const&) = delete;
        Base& operator= (Base const&) = delete;
        Base (Base&&) = delete;
        Base& operator= (Base&&) = delete;
      };
    }
  }
}

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#endif

#define GSPC_WE_PLUGIN_CREATE_SIGNATURE()                                   \
  extern "C" GSPC_DLLEXPORT                                                 \
    std::unique_ptr<::gspc::we::plugin::Base>                               \
      gspc_we_plugin_create ( ::gspc::we::plugin::Context const& context    \
                            , ::gspc::we::plugin::PutToken put_token        \
                            )

GSPC_WE_PLUGIN_CREATE_SIGNATURE();

#define GSPC_WE_PLUGIN_CREATE(_plugin)                  \
  GSPC_WE_PLUGIN_CREATE_SIGNATURE()                     \
  {                                                     \
    return std::unique_ptr<::gspc::we::plugin::Base>    \
      (new _plugin (context, std::move (put_token)));   \
  }

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif
