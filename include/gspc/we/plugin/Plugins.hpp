// Copyright (C) 2019,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/plugin/ID.hpp>
#include <gspc/we/plugin/Plugin.hpp>

#include <filesystem>

#include <unordered_map>



    namespace gspc::we::plugin
    {
      struct Plugins
      {
        ID create (std::filesystem::path, Context const&, PutToken);
        void destroy (ID);

        void before_eval (ID, Context const&);
        void after_eval (ID, Context const&);

      private:
        ID _next_id = ID {0};
        std::unordered_map<ID, Plugin> _;

        decltype (_)::iterator at (ID);
      };
    }
