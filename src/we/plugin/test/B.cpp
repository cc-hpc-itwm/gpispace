// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/plugin/Base.hpp>

#include <string>
#include <utility>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      struct B : public Base
      {
        B (Context const& context, PutToken put_token)
          : _ (::boost::get<std::string> (context.value ({"B"})))
          , _put_token (std::move (put_token))
        {}

        void before_eval (Context const&) override
        {
          _put_token (_, _before++);
        }
        void after_eval (Context const&) override
        {
          _put_token (_, _after++);
        }

      private:
        std::string _;
        PutToken _put_token;
        unsigned _before = 0;
        unsigned _after = 0;
      };
    }
  }
}

GSPC_WE_PLUGIN_CREATE (::gspc::we::plugin::B)
