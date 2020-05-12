#include <we/plugin/Base.hpp>
#include <we/type/stencil_cache.hpp>

#include <util-generic/unreachable.hpp>

#include <fstream>
#include <string>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      struct StencilCache : public Base
      {
        StencilCache (Context const& context, PutToken put_token)
          : _ (context, std::move (put_token))
        {}

        virtual void before_eval (Context const&) override
        {
          FHG_UTIL_UNREACHABLE();
        }

        virtual void after_eval (Context const& context) override
        {
          auto const operation
            (boost::get<std::string> (context.value ({"operation"})));

          if ("ALLOC" == operation)
          {
            _.alloc (context);
          }
          else if ("PREPARED" == operation)
          {
            _.prepared (context);
          }
          else if ("FREE" == operation)
          {
            _.free (context);
          }
          else
          {
            throw std::invalid_argument
              ( ( boost::format ("stencil_cache: Unknown operation %1%")
                % operation
                ).str()
              );
          }
        }

      private:
        ::we::type::stencil_cache _;
      };
    }
  }
}

GSPC_WE_PLUGIN_CREATE (gspc::we::plugin::StencilCache)
