// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <ostream>
#include <string>

#define NO_NAME_MANGLING extern "C"

NO_NAME_MANGLING GSPC_DLLEXPORT void client_with_ostream_logger
  (std::string const&, std::ostream&);
