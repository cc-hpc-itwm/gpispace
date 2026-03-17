// Copyright (C) 2019,2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/plugin/Base.hpp>

#include <gspc/util/dynamic_linking.hpp>

#include <filesystem>

#include <memory>



    namespace gspc::we::plugin
    {
      struct Plugin
      {
        Plugin (std::filesystem::path, Context const&, PutToken);

        void before_eval (Context const&);
        void after_eval (Context const&);

      private:
        util::scoped_dlhandle _dlhandle;
        std::unique_ptr<Base> _;
      };
    }
