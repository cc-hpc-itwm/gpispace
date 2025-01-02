// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <testing/certificates_data.hpp>

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
