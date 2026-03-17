// Copyright (C) 2014-2015,2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/drts/information_to_reattach.hpp>
#include <gspc/drts/private/host_and_port.hpp>

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
