// Copyright (C) 2013-2016,2019-2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/we/loader/Module.hpp>

#include <gspc/we/expr/eval/context.hpp>
#include <gspc/we/loader/api-guard.hpp>
#include <gspc/we/loader/exceptions.hpp>
#include <gspc/testing/printer/we/type/value.hpp>

#include <gspc/drts/worker/context.hpp>
#include <gspc/drts/worker/context_impl.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/version.hpp>

BOOST_AUTO_TEST_CASE (ctor_load_failed)
{
  gspc::testing::require_exception
    ( [] { gspc::we::loader::Module ("<path>"); }
    , gspc::we::loader::module_load_failed
        ( "<path>"
        , "dlopen: <path>: cannot open shared object file: No such file or directory"
        )
    );
}

BOOST_AUTO_TEST_CASE (ctor_failed_exception_from_we_mod_initialize)
{
  gspc::testing::require_exception
    ( [] { gspc::we::loader::Module ("./libinitialize_throws.so"); }
    , gspc::we::loader::module_load_failed
        ( "./libinitialize_throws.so"
        , "initialize_throws"
        )
    );
}

BOOST_AUTO_TEST_CASE (call_not_found)
{
  gspc::we::loader::Module m ("./libempty.so");

  gspc::logging::stream_emitter logger;
  drts::worker::context context
    (drts::worker::context_constructor
      ("noname", (std::set<std::string>()), logger)
    );
  gspc::we::expr::eval::context input;
  gspc::we::expr::eval::context output;

  gspc::testing::require_exception
    ( [&m, &context, &input, &output]
    {
      m.call ( "<name>", &context, input, output
             , std::map<std::string, void*>()
             );
    }
    , gspc::we::loader::function_not_found ("./libempty.so", "<name>")
    );
}

namespace
{
  static int x;

  void inc ( drts::worker::context*
           , gspc::we::expr::eval::context const&
           , gspc::we::expr::eval::context&
           , std::map<std::string, void*> const&
           )
  {
    ++x;
  }
}

BOOST_AUTO_TEST_CASE (call_local)
{
  gspc::we::loader::Module m ("./libempty.so");
  m.add_function ("f", &inc);

  gspc::logging::stream_emitter logger;
  drts::worker::context context
    (drts::worker::context_constructor
      ("noname", (std::set<std::string>()), logger)
    );
  gspc::we::expr::eval::context input;
  gspc::we::expr::eval::context output;

  x=0;

  m.call ("f", &context, input, output, std::map<std::string, void*>());

  BOOST_REQUIRE_EQUAL (x, 1);
}

BOOST_AUTO_TEST_CASE (call_lib)
{
  gspc::we::loader::Module m ("./libanswer.so");

  gspc::logging::stream_emitter logger;
  drts::worker::context context
    (drts::worker::context_constructor
      ("noname", (std::set<std::string>()), logger)
    );
  gspc::we::expr::eval::context input;
  gspc::we::expr::eval::context output;

  m.call ("answer", &context, input, output, std::map<std::string, void*>());

  BOOST_REQUIRE_EQUAL ( output.value ({"out"})
                      , gspc::pnet::type::value::value_type (42L)
                      );
}

BOOST_AUTO_TEST_CASE (duplicate_function)
{
  gspc::we::loader::Module m ("./libempty.so");
  m.add_function ("f", &inc);

  gspc::testing::require_exception
    ( [&m] { m.add_function ("f", &inc); }
    , gspc::we::loader::duplicate_function ("./libempty.so", "f")
    );
}

BOOST_AUTO_TEST_CASE (ensures_library_unloads_properly)
{
  // \note Relies on the library not linking anyone in addition to
  // what is already loaded, so that the not-unloaded set is only
  // exactly the library we know about.

  auto const libempty_nodelete ("./libempty_nodelete.so");

  gspc::testing::require_exception
    ( [&]
      {
        gspc::we::loader::Module
          {gspc::we::loader::RequireModuleUnloadsWithoutRest{}, libempty_nodelete};
      }
    , gspc::we::loader::module_load_failed
        ( libempty_nodelete
        , gspc::we::loader::module_does_not_unload
            (libempty_nodelete, {libempty_nodelete}).what()
        )
    );
}
