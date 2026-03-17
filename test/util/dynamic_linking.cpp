#include <gspc/util/dynamic_linking.hpp>
#include <gspc/util/executable_path.hpp>
#include <gspc/testing/printer/vector.hpp>
#include <gspc/testing/require_exception.hpp>

#include <test/util/dynamic_linking-lib.hpp>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <stdexcept>


  namespace gspc::util
  {
    //! \todo the tests comparing exceptions are probably not very portable…
    BOOST_AUTO_TEST_CASE (missing_file_is_reported)
    {
      gspc::testing::require_exception
        ( []
          {
            std::ignore = scoped_dlhandle
              { std::filesystem::path
                { "this is hopefully - a non/existing \\path"}
              };
          }
        , std::runtime_error ( "dlopen: this is hopefully - a non/existing "
                               "\\path: cannot open shared object file: No "
                               "such file or directory"
                             )
        );
    }

    BOOST_AUTO_TEST_CASE (missing_symbol_is_reported)
    {
      scoped_dlhandle dl {std::filesystem::path {GSPC_UTIL_TESTING_LIB_PATH}};

      gspc::testing::require_exception
        ( [&] { dl.sym<void> ("this symbol hopefully doesn't exist"); }
        , gspc::testing::make_nested
            ( std::runtime_error
                ("get symbol 'this symbol hopefully doesn't exist'")
            , std::runtime_error ( "dlsym: " GSPC_UTIL_TESTING_LIB_PATH
                                   ": undefined symbol: this symbol hopefully "
                                   "doesn't exist"
                                 )
            )
        );
    }

    BOOST_AUTO_TEST_CASE (symbols_are_possible_to_be_retrieved_and_called)
    {
      scoped_dlhandle dl {std::filesystem::path {GSPC_UTIL_TESTING_LIB_PATH}};

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
      scoped_dlhandle dl0 {std::filesystem::path {GSPC_UTIL_TESTING_LIB_PATH}};

      auto variable (FHG_UTIL_SCOPED_DLHANDLE_SYMBOL (dl0, dltest_variable));

      {
        scoped_dlhandle dl1 {std::filesystem::path {GSPC_UTIL_TESTING_LIB_PATH}};

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
        scoped_dlhandle dl {std::filesystem::path {GSPC_UTIL_TESTING_LIB_PATH}};

        auto const during (currently_loaded_libraries());

        BOOST_REQUIRE_NE (before, during);

        BOOST_REQUIRE
          ( std::find (during.begin(), during.end(), GSPC_UTIL_TESTING_LIB_PATH)
          != during.end()
          );
      }

      auto const after (currently_loaded_libraries());

      BOOST_REQUIRE_EQUAL (before, after);
    }
  }
