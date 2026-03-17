// Copyright (C) 2019,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/plugin/Base.hpp>

#include <stdexcept>
#include <string>



    namespace gspc::we::plugin
    {
      struct C : public Base
      {
        C (Context const& context, PutToken put_token)
        {
          put_token
            ( ::boost::get<std::string> (context.value ({"B"}))
            , context.value ({"A"})
            );
        }

        void before_eval (Context const&) override
        {
          throw std::runtime_error ("C::before_eval()");
        }
        void after_eval (Context const&) override
        {
          throw std::runtime_error ("C::after_eval()");
        }
      };
    }



GSPC_WE_PLUGIN_CREATE (::gspc::we::plugin::C)
