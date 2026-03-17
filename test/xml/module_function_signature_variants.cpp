// Copyright (C) 2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests all valid forms of the module `function` attribute
// grammar:
//
//   S -> R F A
//   R -> eps | valid_name       (return port)
//   F -> valid_name             (function name)
//   A -> eps | '(' L ')' | '(' ')'   (argument list)
//   L -> valid_name | valid_name ',' L
//
// The seven variants exercised are:
//
//   "v f1 (phi, x, y)"  return port + arguments
//   "f2 (phi, x, y)"    no return, with arguments
//   "f3"                bare name, no parens, no arguments
//   "f4()"              no return, empty parens
//   "f5 (i)"            no return, one argument
//   "v f6"              return port, no parens
//   "v f7()"            return port, empty parens

#include <boost/test/unit_test.hpp>

#include <gspc/xml/parse/parser.hpp>
#include <gspc/xml/parse/type/function.hpp>
#include <gspc/xml/parse/type/mod.hpp>
#include <gspc/xml/parse/type/net.hpp>
#include <gspc/xml/parse/type/transition.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>

#include <fmt/core.h>

#include <algorithm>
#include <iterator>
#include <list>
#include <optional>
#include <sstream>
#include <string>

namespace
{
  struct parsed_signature
  {
    std::string function;
    std::optional<std::string> port_return;
    std::list<std::string> port_arg;
  };

  [[nodiscard]] auto parse_module_signature
    (std::string const& xml_snippet) -> parsed_signature
  {
    auto const input
      { fmt::format ( "<defun name=\"mod_type\"><net>{}</net></defun>"
                    , xml_snippet
                    )
      };

    auto state {gspc::xml::parse::state::type{}};
    auto input_stream {std::istringstream {input}};
    auto const function {gspc::xml::parse::just_parse (state, input_stream)};

    auto const& net
      { ::boost::get<gspc::xml::parse::type::net_type> (function.content())
      };

    auto const transition_count
      { std::distance
          (std::begin (net.transitions()), std::end (net.transitions()))
      };
    BOOST_REQUIRE_EQUAL (transition_count, 1);

    auto const& transition {*std::begin (net.transitions())};
    auto const& resolved_function {transition.resolved_function()};
    auto const& module
      { ::boost::get<gspc::xml::parse::type::module_type>
          (resolved_function.content())
      };

    return parsed_signature
      { module.function()
      , module.port_return()
      , module.port_arg()
      };
  }
}

BOOST_AUTO_TEST_CASE (module_function_signature_with_return_and_args)
{
  auto const signature
    { parse_module_signature
      ( {R"EOS(
<transition name="f1">
<defun>
  <in name="x" type="double"/>
  <in name="y" type="double"/>
  <in name="phi" type="double"/>
  <out name="phi" type="double"/>
  <out name="v" type="long"/>
  <module name="mod" function="v f1 (phi, x, y)"/>
</defun>
</transition>)EOS"}
      )
    };
  BOOST_CHECK_EQUAL (signature.function, "f1");
  BOOST_CHECK (signature.port_return.has_value());
  BOOST_CHECK_EQUAL (*signature.port_return, "v");
  auto const expected {std::list<std::string> {"phi", "x", "y"}};
  BOOST_CHECK_EQUAL_COLLECTIONS
    ( std::begin (signature.port_arg)
    , std::end (signature.port_arg)
    , std::begin (expected)
    , std::end (expected)
    );
}

BOOST_AUTO_TEST_CASE (module_function_signature_without_return_with_args)
{
  auto const signature
    { parse_module_signature
      ( {R"EOS(
<transition name="f2">
<defun>
  <in name="x" type="double"/>
  <in name="y" type="double"/>
  <in name="phi" type="double"/>
  <out name="phi" type="double"/>
  <module name="mod" function="f2 (phi, x, y)"/>
</defun>
</transition>)EOS"}
      )
    };
  BOOST_CHECK_EQUAL (signature.function, "f2");
  BOOST_CHECK (!signature.port_return.has_value());
  auto const expected {std::list<std::string> {"phi", "x", "y"}};
  BOOST_CHECK_EQUAL_COLLECTIONS
    ( std::begin (signature.port_arg)
    , std::end (signature.port_arg)
    , std::begin (expected)
    , std::end (expected)
    );
}

BOOST_AUTO_TEST_CASE (module_function_signature_bare_name)
{
  auto const signature
    { parse_module_signature
      ( {R"EOS(
<transition name="f3">
<defun>
  <module name="mod" function="f3"/>
</defun>
</transition>)EOS"}
      )
    };
  BOOST_CHECK_EQUAL (signature.function, "f3");
  BOOST_CHECK (!signature.port_return.has_value());
  BOOST_CHECK (signature.port_arg.empty());
}

BOOST_AUTO_TEST_CASE (module_function_signature_empty_parens)
{
  auto const signature
    { parse_module_signature
      ( {R"EOS(
<transition name="f4">
<defun>
  <module name="mod" function="f4()"/>
</defun>
</transition>)EOS"}
      )
    };
  BOOST_CHECK_EQUAL (signature.function, "f4");
  BOOST_CHECK (!signature.port_return.has_value());
  BOOST_CHECK (signature.port_arg.empty());
}

BOOST_AUTO_TEST_CASE (module_function_signature_one_arg)
{
  auto const signature
    { parse_module_signature
      ( {R"EOS(
<transition name="f5">
<defun>
  <in name="i" type="long"/>
  <module name="mod" function="f5 (i)"/>
</defun>
</transition>)EOS"}
      )
    };
  BOOST_CHECK_EQUAL (signature.function, "f5");
  BOOST_CHECK (!signature.port_return.has_value());
  auto const expected {std::list<std::string> {"i"}};
  BOOST_CHECK_EQUAL_COLLECTIONS
    ( std::begin (signature.port_arg)
    , std::end (signature.port_arg)
    , std::begin (expected)
    , std::end (expected)
    );
}

BOOST_AUTO_TEST_CASE (module_function_signature_return_no_parens)
{
  auto const signature
    { parse_module_signature
      ( {R"EOS(
<transition name="f6">
<defun>
  <out name="v" type="long"/>
  <module name="mod" function="v f6"/>
</defun>
</transition>)EOS"}
      )
    };
  BOOST_CHECK_EQUAL (signature.function, "f6");
  BOOST_CHECK (signature.port_return.has_value());
  BOOST_CHECK_EQUAL (*signature.port_return, "v");
  BOOST_CHECK (signature.port_arg.empty());
}

BOOST_AUTO_TEST_CASE (module_function_signature_return_empty_parens)
{
  auto const signature
    { parse_module_signature
      ( {R"EOS(
<transition name="f7">
<defun>
  <out name="v" type="long"/>
  <module name="mod" function="v f7()"/>
</defun>
</transition>)EOS"}
      )
    };
  BOOST_CHECK_EQUAL (signature.function, "f7");
  BOOST_CHECK (signature.port_return.has_value());
  BOOST_CHECK_EQUAL (*signature.port_return, "v");
  BOOST_CHECK (signature.port_arg.empty());
}
