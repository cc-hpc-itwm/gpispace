// Copyright (C) 2019,2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/testing/certificates_data.hpp>

#include <fmt/ostream.h>
#include <fmt/std.h>

namespace gspc
{
  namespace testing
  {
    Certificates no_certs()
    {
      return {};
    }
    Certificates yes_certs()
    {
      return Certificates
        { std::filesystem::path {GSPC_SSL_CERTIFICATES_FOR_TESTS}
        };
    }
  }

  auto operator<<
    ( std::ostream& os
    , Certificates const& certificates
    ) -> std::ostream&
  {
    fmt::print (os, "{}", certificates.path);

    return os;
  }
}
