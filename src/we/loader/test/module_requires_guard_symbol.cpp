#define BOOST_TEST_MODULE module_requires_guard_symbol

#include <boost/test/unit_test.hpp>

#include <we/loader/Module.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/format.hpp>

BOOST_AUTO_TEST_CASE
  (module_requires_guard_symbol_defined_in_executing_binary)
{
#define XSTR(x) STR(x)
#define STR(x) #x
  fhg::util::testing::require_exception<std::runtime_error>
    ( [] { we::loader::Module ("./libempty_not_linked_with_pnet.so"); }
    , ( boost::format
        ( "could not load module './libempty_not_linked_with_pnet.so':"
          " ./libempty_not_linked_with_pnet.so: undefined symbol: %1%"
        )
      % XSTR (WE_GUARD_SYMBOL)
      ).str()
    );
#undef STR
#undef XSTR
}
