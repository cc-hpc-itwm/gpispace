// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/detail/Installation.hpp>

#include <util-generic/executable_path.hpp>
#include <util-generic/getenv.hpp>

namespace iml
{
  namespace detail
  {
    namespace
    {
      template<int N>
        ::boost::filesystem::path nth_parent_path (::boost::filesystem::path path)
      {
        for (int i (0); i < N; ++i)
        {
          path = path.parent_path();
        }
        return path;
      }

      ::boost::filesystem::path installation_prefix()
      {
        auto const override_path
          (fhg::util::getenv ("IML_TESTING_OVERRIDE_INSTALLATION_PREFIX"));
        if (override_path)
        {
          return *override_path;
        }

        return nth_parent_path<IML_INSTALLATION_SENTINEL_SUBDIR_COUNT>
          (fhg::util::executable_path (&installation_prefix));
      }
    }

    Installation::Installation()
      : server_binary (installation_prefix() / "libexec" / "iml" / "iml-gpi-server")
      , rifd_binary (installation_prefix() / "libexec" / "iml" / "iml-rifd")
    {}
  }
}
