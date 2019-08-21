#include <we/plugin/Base.hpp>

#include <fstream>
#include <string>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      struct Log : public Base
      {
        GSPC_WE_PLUGIN_CONSTRUCTOR (Log, context,)
          : _out
            ( boost::get<std::string>
                (context.value ({"plugin", "Log", "file"}))
            , std::ios::app
            )
        {}

        virtual void before_eval (Context const& context) override
        {
          _out << "before_eval\n" << context << '\n';
        }
        virtual void after_eval (Context const& context) override
        {
          _out << "after_eval\n" << context << '\n';
        }

      private:
        std::ofstream _out;
      };
    }
  }
}

GSPC_WE_PLUGIN_CREATE (gspc::we::plugin::Log)
