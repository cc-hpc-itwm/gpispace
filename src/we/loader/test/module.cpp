// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE module
#include <boost/test/unit_test.hpp>

#include <we/loader/Module.hpp>

#include <we/loader/api-guard.hpp>
#include <we/loader/exceptions.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/boost/test/require_exception.hpp>

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/version.hpp>

namespace
{
  void ctor (std::string const& name, std::string const& path)
  {
    (void) (we::loader::Module (name, path));
  }
}

BOOST_AUTO_TEST_CASE (ctor_failed_not_found)
{
  fhg::util::boost::test::require_exception<we::loader::ModuleLoadFailed>
    ( boost::bind (&ctor, "name", "<path>")
    , "we::loader::ModuleLoadFailed"
    , "could not load module 'name' from '<path>': <path>:"
      " cannot open shared object file: No such file or directory"
    );
}

BOOST_AUTO_TEST_CASE (ctor_failed_exception_from_we_mod_initialize)
{
  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&ctor, "a", "./libinitialize_throws.so")
    , "std::runtime_error"
    , "initialize_throws"
    );
}

BOOST_AUTO_TEST_CASE (ctor_failed_bad_boost_version)
{
#define XSTR(x) STR(x)
#define STR(x) #x
  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&ctor, "a", "./libempty_not_linked_with_pnet.so")
    , "std::runtime_error"
    , ( boost::format
        ( "could not load module 'a' from './libempty_not_linked_with_pnet.so':"
          " ./libempty_not_linked_with_pnet.so: undefined symbol: %1%"
        )
      % XSTR (WE_GUARD_SYMBOL)
      ).str()
    );
#undef STR
#undef XSTR
}

BOOST_AUTO_TEST_CASE (ctor_okay_name_path)
{
  we::loader::Module const m ("name", "./libempty.so");

  BOOST_REQUIRE_EQUAL (m.name(), "empty");
  BOOST_REQUIRE_EQUAL (m.path(), "./libempty.so");
}
