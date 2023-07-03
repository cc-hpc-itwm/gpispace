// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/printer/generic.hpp>

#include <boost/lexical_cast.hpp>

#define FHG_UTIL_TESTING_REQUIRE_PRINTED_AS(text_, ...)                   \
  BOOST_REQUIRE_EQUAL                                                     \
    ( text_                                                               \
    , ::boost::lexical_cast<std::string>                                    \
        (FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (__VA_ARGS__))             \
    )
