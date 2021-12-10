// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
