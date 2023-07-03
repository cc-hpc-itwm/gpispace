// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <drts/certificates.hpp>

#include <boost/ref.hpp>
#include <boost/test/data/monomorphic.hpp>

namespace gspc
{
  namespace testing
  {
    Certificates no_certs();
    Certificates yes_certs();
  }
}

#define certificates_data                                               \
  ::boost::unit_test::data::make                                          \
    ({::gspc::testing::no_certs(), ::gspc::testing::yes_certs()})
