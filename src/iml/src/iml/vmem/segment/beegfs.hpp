// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/filesystem/path.hpp>

#include <stdexcept>

namespace fhg
{
  namespace iml
  {
    namespace vmem
    {
      namespace segment
      {
        namespace beegfs
        {
          struct requirements_not_met : std::runtime_error
          {
            requirements_not_met (::boost::filesystem::path const& path)
              : std::runtime_error
                  ("BeeGFS segment requirements not met for " + path.string())
            {}
          };

          void check_requirements (::boost::filesystem::path const&);
        }
      }
    }
  }
}
