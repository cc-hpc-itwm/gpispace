// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/loader/Module.hpp>
#include <we/loader/api-guard.hpp>
#include <we/loader/exceptions.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <fmt/core.h>

BOOST_AUTO_TEST_CASE (ctor_failed_bad_boost_version)
{
#define XSTR(x) STR(x)
#define STR(x) #x
  fhg::util::testing::require_exception
    ( [] { we::loader::Module ("./libempty_not_linked_with_pnet.so"); }
    , we::loader::module_load_failed
        ( "./libempty_not_linked_with_pnet.so"
        , fmt::format
            ( "dlopen: ./libempty_not_linked_with_pnet.so: undefined symbol: {}"
            , XSTR (WE_GUARD_SYMBOL)
            )
        )
    );
#undef STR
#undef XSTR
}
