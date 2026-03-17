// Copyright (C) 2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/detail/Installation.hpp>

#include <gspc/util/executable_path.hpp>

#include <cstdlib>


  namespace gspc::iml::detail
  {
    namespace
    {
      template<int N>
        std::filesystem::path nth_parent_path (std::filesystem::path path)
      {
        for (int i (0); i < N; ++i)
        {
          path = path.parent_path();
        }
        return path;
      }

      std::filesystem::path compute_installation_prefix()
      {
        auto const override_path
          (std::getenv ("IML_TESTING_OVERRIDE_INSTALLATION_PREFIX"));
        if (override_path)
        {
          return override_path;
        }

        return std::filesystem::path
          { nth_parent_path<IML_INSTALLATION_SENTINEL_SUBDIR_COUNT>
              ( gspc::util::executable_path
                  (&compute_installation_prefix)
              )
          };
      }
    }

    Installation::Installation()
      : installation_prefix (compute_installation_prefix())
      , server_binary
          ( installation_prefix
          / "libexec" / "iml" / "gspc-iml-gpi-server"
          )
    {}
  }
