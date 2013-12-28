// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE module
#include <boost/test/unit_test.hpp>

#include <we/loader/Module.hpp>

#include <we/expr/eval/context.hpp>
#include <we/loader/api-guard.hpp>
#include <we/loader/exceptions.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <gspc/drts/context.hpp>

#include <fhg/util/boost/test/require_exception.hpp>

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/version.hpp>

namespace
{
  void ctor (std::string const& path)
  {
    (void) (we::loader::Module (path));
  }
}

BOOST_AUTO_TEST_CASE (ctor_load_failed)
{
  fhg::util::boost::test::require_exception<we::loader::module_load_failed>
    ( boost::bind (&ctor, "<path>")
    , "we::loader::module_load_failed"
    , "could not load module '<path>': <path>:"
      " cannot open shared object file: No such file or directory"
    );
}

BOOST_AUTO_TEST_CASE (ctor_failed_exception_from_we_mod_initialize)
{
  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&ctor, "./libinitialize_throws.so")
    , "std::runtime_error"
    , "initialize_throws"
    );
}

BOOST_AUTO_TEST_CASE (ctor_failed_bad_boost_version)
{
#define XSTR(x) STR(x)
#define STR(x) #x
  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&ctor, "./libempty_not_linked_with_pnet.so")
    , "std::runtime_error"
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

BOOST_AUTO_TEST_CASE (ctor_okay_name_path)
{
  we::loader::Module const m ("./libempty.so");

  BOOST_REQUIRE_EQUAL (m.name(), "empty");
  BOOST_REQUIRE_EQUAL (m.path(), "./libempty.so");
}

BOOST_AUTO_TEST_CASE (set_name)
{
  we::loader::Module m ("./libempty.so");
  m.name ("name");

  BOOST_REQUIRE_EQUAL (m.name(), "name");
}

BOOST_AUTO_TEST_CASE (call_not_found)
{
  we::loader::Module m ("./libempty.so");

  gspc::drts::context context ((std::list<std::string>()));
  expr::eval::context input;
  expr::eval::context output;

  fhg::util::boost::test::require_exception<we::loader::function_not_found>
    ( boost::bind ( &we::loader::Module::call, &m
                  , "<name>", &context, input, output
                  )
    , "we::loader::function_not_found"
    , "function 'empty::<name>' not found"
    );
}

namespace
{
  static int x;

  void inc ( gspc::drts::context*
           , expr::eval::context const&
           , expr::eval::context&
           )
  {
    ++x;
  }
}

BOOST_AUTO_TEST_CASE (call_local)
{
  we::loader::Module m ("./libempty.so");
  m.add_function ("f", &inc);

  gspc::drts::context context ((std::list<std::string>()));
  expr::eval::context input;
  expr::eval::context output;

  x=0;

  m.call ("f", &context, input, output);

  BOOST_REQUIRE_EQUAL (x, 1);
}

BOOST_AUTO_TEST_CASE (call_lib)
{
  we::loader::Module m ("./libanswer.so");

  gspc::drts::context context ((std::list<std::string>()));
  expr::eval::context input;
  expr::eval::context output;

  m.call ("answer", &context, input, output);

  BOOST_REQUIRE_EQUAL ( output.value ("out")
                      , pnet::type::value::value_type (42L)
                      );
}

BOOST_AUTO_TEST_CASE (duplicate_function)
{
  we::loader::Module m ("./libempty.so");
  m.add_function ("f", &inc);

  fhg::util::boost::test::require_exception<we::loader::duplicate_function>
    ( boost::bind ( &we::loader::Module::add_function, &m, "f", inc)
    , "we::loader::duplicate_function"
    , "duplicate function 'empty::f'"
    );
}

BOOST_AUTO_TEST_CASE (finalize_throw_ignored)
{
  we::loader::Module m ("./libfinalize_throws.so");
}
