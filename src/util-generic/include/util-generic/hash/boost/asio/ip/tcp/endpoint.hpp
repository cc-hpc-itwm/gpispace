// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/hash/combined_hash.hpp>

#include <boost/asio/ip/tcp.hpp>

FHG_UTIL_MAKE_COMBINED_STD_HASH ( ::boost::asio::ip::tcp::endpoint
                                , endpoint
                                , endpoint.address().to_string()
                                , endpoint.port()
                                )
