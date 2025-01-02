// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/program_options/variables_map.hpp>

namespace iml_test
{
  void set_iml_vmem_socket_path_for_localhost
    (::boost::program_options::variables_map&);
}
