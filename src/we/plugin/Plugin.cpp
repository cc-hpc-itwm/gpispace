#include <we/plugin/Plugin.hpp>

#include <utility>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      Plugin::Plugin ( boost::filesystem::path path
                     , Context const& context
                     , PutToken put_token
                     )
        : _dlhandle (path)
        , _ ( FHG_UTIL_SCOPED_DLHANDLE_SYMBOL (_dlhandle, gspc_we_plugin_create)
                (context, std::move (put_token))
            )
      {}

      void Plugin::before_eval (Context const& context)
      {
        _->before_eval (context);
      }
      void Plugin::after_eval (Context const& context)
      {
        _->after_eval (context);
      }
    }
  }
}
