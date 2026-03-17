// Copyright (C) 2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <stdexcept>





        namespace gspc::iml::vmem::segment::beegfs
        {
          struct requirements_not_met : std::runtime_error
          {
            requirements_not_met (std::filesystem::path const& path)
              : std::runtime_error
                  ("BeeGFS segment requirements not met for " + path.string())
            {}
          };

          void check_requirements (std::filesystem::path const&);
        }
