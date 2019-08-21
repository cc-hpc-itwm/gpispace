#include <we/plugin/Base.hpp>

#include <stdexcept>
#include <string>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      struct C : public Base
      {
        GSPC_WE_PLUGIN_CONSTRUCTOR (C, context, put_token)
        {
          put_token
            ( boost::get<std::string> (context.value ({"B"}))
            , context.value ({"A"})
            );
        }

        virtual void before_eval (Context const&) override
        {
          throw std::runtime_error ("C::before_eval()");
        }
        virtual void after_eval (Context const&) override
        {
          throw std::runtime_error ("C::after_eval()");
        }
      };
    }
  }
}

GSPC_WE_PLUGIN_CREATE (::gspc::we::plugin::C)
