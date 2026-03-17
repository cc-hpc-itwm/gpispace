// Copyright (C) 2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/loader/Module.hpp>
#include <gspc/we/loader/api-guard.hpp>
#include <gspc/we/loader/exceptions.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <fmt/core.h>

BOOST_AUTO_TEST_CASE (ctor_failed_bad_boost_version)
{
#define XSTR(x) STR(x)
#define STR(x) #x
  gspc::testing::require_exception
    ( [] { gspc::we::loader::Module ("./libempty_not_linked_with_pnet.so"); }
    , gspc::we::loader::module_load_failed
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
