// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/type/memory_buffer.hpp>

#include <xml/parse/parser.hpp>

#include <fhg/util/xml.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/optional.hpp>
#include <boost/test/data/test_case.hpp>

#include <sstream>
#include <string>
#include <vector>
#include <fmt/core.h>

namespace
{
  std::vector<::boost::optional<bool>> tribool_values()
  {
    return {{false, false}, {true, false}, {true, true}};
  }
}

BOOST_DATA_TEST_CASE
  (pass_context_is_generating_correctly, tribool_values(), pass_context)
{
  std::istringstream input_stream
    ( fmt::format
              (R"EOS(
<defun name="f">
  <module name="m" function="f()" {}>
    <code/>
  </module>
</defun>)EOS"
          , (  pass_context &&  *pass_context ? "pass_context=\"true\""
            :  pass_context && !*pass_context ? "pass_context=\"false\""
            : !pass_context                   ? ""
                                              : "<unreachable>"
            )
          )
    );

  auto const expected_generated_cpp
    ( fmt::format
              (R"EOS(#include <we/loader/macros.hpp>

#include <pnetc/op/m/f.hpp>

namespace pnetc
{{
  namespace op
  {{
    namespace m
    {{
      static void f
        ( drts::worker::context *{0}
        , expr::eval::context const&
        , expr::eval::context&
        , std::map<std::string, void*> const&
        )
      {{
        ::pnetc::op::m::f ({0});
      }}
    }}
  }}
}}
WE_MOD_INITIALIZE_START()
{{
  WE_REGISTER_FUN_AS (::pnetc::op::m::f,"f");
}}
WE_MOD_INITIALIZE_END()
)EOS"
          , (  pass_context &&  *pass_context ? "_pnetc_context"
            :  pass_context && !*pass_context ? ""
            : !pass_context                   ? ""
                                              : "<unreachable>"
            )
          )
    );

  fhg::util::temporary_path const scoped_gen_output_path;
  ::boost::filesystem::path const gen_output_path
    (scoped_gen_output_path);

  xml::parse::state::type state;
  state.path_to_cpp() = gen_output_path.string();

  xml::parse::generate_cpp
    (xml::parse::just_parse (state, input_stream), state);

  BOOST_REQUIRE_EQUAL
    ( fhg::util::read_file<std::string>
        (gen_output_path / "pnetc" / "op" / "m.cpp")
    , expected_generated_cpp
    );
}


BOOST_DATA_TEST_CASE
  (pass_context_is_linking_correctly, tribool_values(), pass_context)
{
  std::istringstream input_stream
    ( fmt::format
              (R"EOS(
<defun name="f">
  <module name="m" function="f()" {}>
    <code/>
  </module>
</defun>)EOS"
          , (  pass_context &&  *pass_context ? "pass_context=\"true\""
            :  pass_context && !*pass_context ? "pass_context=\"false\""
            : !pass_context                   ? ""
                                              : "<unreachable>"
            )
          )
    );

  auto const expected_generated_makefile
    (  pass_context &&  *pass_context ? "LDFLAGS_m += -ldrts-context\n"
    :  pass_context && !*pass_context ? ""
    : !pass_context                   ? ""
                                      : "<unreachable>"
    );

  fhg::util::temporary_path const scoped_gen_output_path;
  ::boost::filesystem::path const gen_output_path
    (scoped_gen_output_path);

  xml::parse::state::type state;
  state.path_to_cpp() = gen_output_path.string();

  xml::parse::generate_cpp
    (xml::parse::just_parse (state, input_stream), state);

  BOOST_REQUIRE_EQUAL
    ( fhg::util::read_file<std::string>
        (gen_output_path / "Makefile.LDFLAGS_m")
    , expected_generated_makefile
    );
}
