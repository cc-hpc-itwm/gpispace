// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <drts/information_to_reattach.hpp>
#include <drts/private/host_and_port.hpp>

#include <string>

namespace gspc
{
  struct information_to_reattach::implementation
  {
    explicit
    implementation (host_and_port_type const&);

    explicit
    implementation (std::string const&);

    std::string to_string() const;

    host_and_port_type const& endpoint() const;
  private:
    host_and_port_type _endpoint;
  };
}
