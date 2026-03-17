// Copyright (C) 2014-2016,2018,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/memory_buffer.hpp>

#include <gspc/xml/parse/parser.hpp>

#include <gspc/util/xml.hpp>
#include <gspc/util/read_file.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/optional.hpp>
#include <gspc/testing/random/string.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <fmt/core.h>
#include <optional>

BOOST_AUTO_TEST_CASE (name_is_stored)
{
  std::string const name (gspc::testing::random_string());

  BOOST_REQUIRE_EQUAL
    ( name
    , gspc::xml::parse::type::memory_buffer_type
      ( gspc::xml::parse::util::position_type
        (nullptr, nullptr, gspc::testing::random_string())
      , name
      , gspc::testing::random_string()
      , gspc::testing::random_string()
      , std::nullopt
      , gspc::we::type::property::type()
      ).name()
    );
}

BOOST_AUTO_TEST_CASE (size_is_stored)
{
  std::string const size (gspc::testing::random_string());

  BOOST_REQUIRE_EQUAL
    ( size
    , gspc::xml::parse::type::memory_buffer_type
      ( gspc::xml::parse::util::position_type
        (nullptr, nullptr, gspc::testing::random_string())
      , gspc::testing::random_string()
      , size
      , gspc::testing::random_string()
      , std::nullopt
      , gspc::we::type::property::type()
      ).size()
    );
}

BOOST_AUTO_TEST_CASE (alignment_is_stored)
{
  auto const alignment (gspc::testing::random_string());

  BOOST_REQUIRE_EQUAL
    ( alignment
    , gspc::xml::parse::type::memory_buffer_type
      ( gspc::xml::parse::util::position_type
        (nullptr, nullptr, gspc::testing::random_string())
      , gspc::testing::random_string()
      , gspc::testing::random_string()
      , alignment
      , std::nullopt
      , gspc::we::type::property::type()
      ).alignment()
    );
}

namespace
{
  std::vector<std::optional<bool>> tribool_values()
  {
    return {std::nullopt, std::optional{false}, std::optional{true}};
  }
}

BOOST_DATA_TEST_CASE (read_only_is_stored, tribool_values(), read_only)
{
  BOOST_REQUIRE_EQUAL
    ( read_only
    , gspc::xml::parse::type::memory_buffer_type
        ( gspc::xml::parse::util::position_type
          (nullptr, nullptr, gspc::testing::random_string())
        , gspc::testing::random_string()
        , gspc::testing::random_string()
        , gspc::testing::random_string()
        , read_only
        , gspc::we::type::property::type()
        ).read_only()
      );
}

BOOST_AUTO_TEST_CASE (name_is_unique_key)
{
  std::string const name (gspc::testing::random_string());

  BOOST_REQUIRE_EQUAL
    ( name
    , gspc::xml::parse::type::memory_buffer_type
      ( gspc::xml::parse::util::position_type
        (nullptr, nullptr, gspc::testing::random_string())
      , name
      , gspc::testing::random_string()
      , gspc::testing::random_string()
      , std::nullopt
      , gspc::we::type::property::type()
      ).unique_key()
    );
}

BOOST_DATA_TEST_CASE (dump, tribool_values(), read_only)
{
  std::string const name (gspc::testing::random_identifier());
  std::string const size (gspc::testing::random_string_without_zero());
  std::string const alignment
    (gspc::testing::random_string_without_zero());

  gspc::xml::parse::type::memory_buffer_type mb
    ( gspc::xml::parse::util::position_type
        (nullptr, nullptr, gspc::testing::random_string())
    , name
    , size
    , alignment
    , read_only
    , gspc::we::type::property::type()
    );

  std::ostringstream oss;

  gspc::util::xml::xmlstream s (oss);

  gspc::xml::parse::type::dump::dump (s, mb);

  const std::string expected
    ( fmt::format (R"EOS(<memory-buffer name="{0}"{2}>
  <size>{1}</size>
  <alignment>{3}</alignment>
</memory-buffer>)EOS"
      , name
      , size
      , ( !read_only.has_value() ? std::string()
        : fmt::format ( R"EOS( read-only="{}")EOS"
                      , *read_only ? "true" : "false"
                      )
        )
      , alignment
      )
    );

  BOOST_REQUIRE_EQUAL (expected, oss.str());
}


BOOST_DATA_TEST_CASE
  (read_only_is_generating_correctly, tribool_values(), read_only)
{
  std::istringstream input_stream
    ( fmt::format
              (R"EOS(
<defun name="f">
  <memory-buffer name="b" {}>
    <size>1</size>
  </memory-buffer>
  <module name="m" function="f (b)">
    <code/>
  </module>
</defun>)EOS"
         ,  (  read_only &&  *read_only ? "read-only=\"true\""
            :  read_only && !*read_only ? "read-only=\"false\""
            : !read_only                ? ""
                                        : "<unreachable>"
            )
          )
    );

  auto const expected_generated_cpp
    ( fmt::format
              (R"EOS(#include <pnetc/op/m/f.hpp>

namespace pnetc
{{
  namespace op
  {{
    namespace m
    {{
      void f
        ( {} b
        )
      {{
#line 7 "<stdin>"

      }}
    }}
  }}
}})EOS"
          , (  read_only &&  *read_only ? "void const*"
            :  read_only && !*read_only ? "void*"
            : !read_only                ? "void*"
                                        : "<unreachable>"
            )
          )
    );

  gspc::testing::temporary_path const scoped_gen_output_path;
  std::filesystem::path const gen_output_path
    (scoped_gen_output_path);

  gspc::xml::parse::state::type state;
  state.path_to_cpp() = gen_output_path.string();

  gspc::xml::parse::generate_cpp
    (gspc::xml::parse::just_parse (state, input_stream), state);

  BOOST_REQUIRE_EQUAL
    ( gspc::util::read_file<std::string>
        (gen_output_path / "pnetc" / "op" / "m" / "f.cpp")
    , expected_generated_cpp
    );
}
