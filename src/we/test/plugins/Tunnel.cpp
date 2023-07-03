// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/plugin/Base.hpp>

#include <stdexcept>
#include <string>
#include <utility>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      struct Tunnel : public Base
      {
        Tunnel (Context const& context, PutToken put_token)
          : _place_name
            ( ::boost::get<std::string>
                (context.value ({"plugin", "Tunnel", "place_name"}))
            )
          , _context_key
            ( ::boost::get<std::string>
                (context.value ({"plugin", "Tunnel", "context_key"}))
            )
          , _put_token (std::move (put_token))
        {}

        void before_eval (Context const&) override
        {
          throw std::logic_error ("Tunnel::before_eval");
        }
        void after_eval (Context const& context) override
        {
          return _put_token (_place_name, context.value ({_context_key}));
        }

      private:
        std::string _place_name;
        std::string _context_key;
        PutToken _put_token;
      };
    }
  }
}

GSPC_WE_PLUGIN_CREATE (gspc::we::plugin::Tunnel)
