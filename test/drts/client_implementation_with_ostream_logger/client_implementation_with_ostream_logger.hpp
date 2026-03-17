// Copyright (C) 2015,2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <ostream>
#include <string>

#define NO_NAME_MANGLING extern "C"

NO_NAME_MANGLING GSPC_EXPORT void client_with_ostream_logger
  (std::string const&, std::ostream&);
