#pragma once

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
      using PutToken =
        std::function<void (std::string, ::pnet::type::value::value_type)>;

      struct Base
      {
        //! \note created via Plugin::Plugin (Context const&, PutToken);

        virtual void before_eval (Context const&) = 0;
        virtual void after_eval (Context const&) = 0;

        virtual ~Base() = default;
      };
    }
  }
}

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#endif

#define GSPC_WE_PLUGIN_CREATE_SIGNATURE()                                   \
  extern "C" [[gnu::visibility ("default")]]                                \
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

#define GSPC_WE_PLUGIN_CONSTRUCTOR(_plugin,_context,_put_token) \
  _plugin ( ::gspc::we::plugin::Context const& _context         \
          , ::gspc::we::plugin::PutToken _put_token      \
          )

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif
