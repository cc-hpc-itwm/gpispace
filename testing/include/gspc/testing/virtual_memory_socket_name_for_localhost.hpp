// Copyright (C) 2014-2015,2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/program_options.hpp>

namespace gspc::testing
{
  void set_virtual_memory_socket_name_for_localhost
    (::boost::program_options::variables_map&);
}
