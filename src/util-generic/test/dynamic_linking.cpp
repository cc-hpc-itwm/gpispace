// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <util-generic/dynamic_linking.hpp>
#include <util-generic/executable_path.hpp>
#include <util-generic/test/dynamic_linking-lib.hpp>
#include <util-generic/testing/printer/vector.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    //! \todo the tests comparing exceptions are probably not very portable…
    BOOST_AUTO_TEST_CASE (missing_file_is_reported)
    {
      testing::require_exception
        ( [] { scoped_dlhandle {"this is hopefully - a non/existing \\path"}; }
        , std::runtime_error ( "dlopen: this is hopefully - a non/existing "
                               "\\path: cannot open shared object file: No "
                               "such file or directory"
                             )
        );
    }

    BOOST_AUTO_TEST_CASE (missing_symbol_is_reported)
    {
      scoped_dlhandle dl {FHG_UTIL_TESTING_LIB_PATH};

      testing::require_exception
        ( [&] { dl.sym<void> ("this symbol hopefully doesn't exist"); }
        , testing::make_nested
            ( std::runtime_error
                ("get symbol 'this symbol hopefully doesn't exist'")
            , std::runtime_error ( "dlsym: " FHG_UTIL_TESTING_LIB_PATH
                                   ": undefined symbol: this symbol hopefully "
                                   "doesn't exist"
                                 )
            )
        );
    }

    BOOST_AUTO_TEST_CASE (symbols_are_possible_to_be_retrieved_and_called)
    {
      scoped_dlhandle dl {FHG_UTIL_TESTING_LIB_PATH};

      auto variable (FHG_UTIL_SCOPED_DLHANDLE_SYMBOL (dl, dltest_variable));
      auto setter (FHG_UTIL_SCOPED_DLHANDLE_SYMBOL (dl, dltest_setter));
      auto getter (FHG_UTIL_SCOPED_DLHANDLE_SYMBOL (dl, dltest_getter));

      BOOST_REQUIRE_EQUAL (*variable, -1);
      *variable = 1;
      BOOST_REQUIRE_EQUAL (getter(), 1);
      setter (2);
      BOOST_REQUIRE_EQUAL (getter(), 2);
      BOOST_REQUIRE_EQUAL (*variable, 2);
    }

    BOOST_AUTO_TEST_CASE (can_have_multiple_handles)
    {
      scoped_dlhandle dl0 {FHG_UTIL_TESTING_LIB_PATH};

      auto variable (FHG_UTIL_SCOPED_DLHANDLE_SYMBOL (dl0, dltest_variable));

      {
        scoped_dlhandle dl1 {FHG_UTIL_TESTING_LIB_PATH};

        auto setter (FHG_UTIL_SCOPED_DLHANDLE_SYMBOL (dl1, dltest_setter));

        setter (1);
        BOOST_REQUIRE_EQUAL (*variable, 1);
      }

      BOOST_REQUIRE_EQUAL (*variable, 1);
    }

    BOOST_AUTO_TEST_CASE (loading_and_unloading_a_library_is_observable)
    {
      auto const before (currently_loaded_libraries());

      {
        scoped_dlhandle dl {FHG_UTIL_TESTING_LIB_PATH};

        auto const during (currently_loaded_libraries());

        BOOST_REQUIRE_NE (before, during);

        BOOST_REQUIRE
          ( std::find (during.begin(), during.end(), FHG_UTIL_TESTING_LIB_PATH)
          != during.end()
          );
      }

      auto const after (currently_loaded_libraries());

      BOOST_REQUIRE_EQUAL (before, after);
    }
  }
}
