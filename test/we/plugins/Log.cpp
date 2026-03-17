// Copyright (C) 2019,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/plugin/Base.hpp>

#include <fstream>
#include <string>



    namespace gspc::we::plugin
    {
      struct Log : public Base
      {
        Log (Context const& context, PutToken)
          : _out
            ( ::boost::get<std::string>
                (context.value ({"plugin", "Log", "file"}))
            , std::ios::app
            )
        {}

        void before_eval (Context const& context) override
        {
          _out << "before_eval\n" << context << '\n';
        }
        void after_eval (Context const& context) override
        {
          _out << "after_eval\n" << context << '\n';
        }

      private:
        std::ofstream _out;
      };
    }



GSPC_WE_PLUGIN_CREATE (gspc::we::plugin::Log)
