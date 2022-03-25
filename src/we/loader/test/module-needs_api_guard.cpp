// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <we/loader/Module.hpp>
#include <we/loader/api-guard.hpp>
#include <we/loader/exceptions.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/format.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (ctor_failed_bad_boost_version)
{
#define XSTR(x) STR(x)
#define STR(x) #x
  fhg::util::testing::require_exception
    ( [] { we::loader::Module ("./libempty_not_linked_with_pnet.so"); }
    , we::loader::module_load_failed
        ( "./libempty_not_linked_with_pnet.so"
        , ( ::boost::format
              ("dlopen: ./libempty_not_linked_with_pnet.so: undefined symbol: %1%")
          % XSTR (WE_GUARD_SYMBOL)
          ).str()
        )
    );
#undef STR
#undef XSTR
}
