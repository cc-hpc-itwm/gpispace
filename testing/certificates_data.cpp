// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <testing/certificates_data.hpp>

namespace gspc
{
  namespace testing
  {
    Certificates no_certs()
    {
      return ::boost::none;
    }
    Certificates yes_certs()
    {
      return Certificates (GSPC_SSL_CERTIFICATES_FOR_TESTS);
    }
  }
}
