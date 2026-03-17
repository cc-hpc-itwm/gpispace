// Copyright (C) 2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/testing/printer/generic.hpp>

#include <boost/lexical_cast.hpp>

#define GSPC_TESTING_REQUIRE_PRINTED_AS(text_, ...)                            \
  BOOST_REQUIRE_EQUAL                                                          \
    ( text_                                                                    \
    , ::boost::lexical_cast<std::string>                                       \
        (GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (__VA_ARGS__))                 \
    )
