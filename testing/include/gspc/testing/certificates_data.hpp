// Copyright (C) 2019,2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/drts/certificates.hpp>

#include <iostream>

namespace gspc::testing
{
  Certificates no_certs();
  Certificates yes_certs();
}

namespace gspc
{
  auto operator<< (std::ostream&, Certificates const&) -> std::ostream&;
}

#define certificates_data                                               \
  ::boost::unit_test::data::make                                          \
    ({::gspc::testing::no_certs(), ::gspc::testing::yes_certs()})
