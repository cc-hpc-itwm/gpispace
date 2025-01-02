// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#if defined (__GNUC__) && (__GNUC__ >= 7)
#  define FHG_UTIL_FALLTHROUGH [[fallthrough]]
#else
#  include <boost/config.hpp>
#  define FHG_UTIL_FALLTHROUGH BOOST_FALLTHROUGH
#endif
