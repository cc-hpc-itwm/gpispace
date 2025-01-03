// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <drts/certificates.hpp>

#include <boost/ref.hpp>
#include <boost/test/data/monomorphic.hpp>

#include <iostream>

namespace gspc
{
  namespace testing
  {
    Certificates no_certs();
    Certificates yes_certs();
  }

  auto operator<< (std::ostream&, Certificates const&) -> std::ostream&;
}

#define certificates_data                                               \
  ::boost::unit_test::data::make                                          \
    ({::gspc::testing::no_certs(), ::gspc::testing::yes_certs()})
