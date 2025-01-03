// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/type/memory_buffer.hpp>

#include <xml/parse/parser.hpp>

#include <fhg/util/xml.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/random/string.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <fmt/core.h>

BOOST_AUTO_TEST_CASE (name_is_stored)
{
  std::string const name (fhg::util::testing::random_string());

  BOOST_REQUIRE_EQUAL
    ( name
    , xml::parse::type::memory_buffer_type
      ( xml::parse::util::position_type
        (nullptr, nullptr, fhg::util::testing::random_string())
      , name
      , fhg::util::testing::random_string()
      , fhg::util::testing::random_string()
      , ::boost::none
      , we::type::property::type()
      ).name()
    );
}

BOOST_AUTO_TEST_CASE (size_is_stored)
{
  std::string const size (fhg::util::testing::random_string());

  BOOST_REQUIRE_EQUAL
    ( size
    , xml::parse::type::memory_buffer_type
      ( xml::parse::util::position_type
        (nullptr, nullptr, fhg::util::testing::random_string())
      , fhg::util::testing::random_string()
      , size
      , fhg::util::testing::random_string()
      , ::boost::none
      , we::type::property::type()
      ).size()
    );
}

BOOST_AUTO_TEST_CASE (alignment_is_stored)
{
  auto const alignment (fhg::util::testing::random_string());

  BOOST_REQUIRE_EQUAL
    ( alignment
    , xml::parse::type::memory_buffer_type
      ( xml::parse::util::position_type
        (nullptr, nullptr, fhg::util::testing::random_string())
      , fhg::util::testing::random_string()
      , fhg::util::testing::random_string()
      , alignment
      , ::boost::none
      , we::type::property::type()
      ).alignment()
    );
}

namespace
{
  std::vector<::boost::optional<bool>> tribool_values()
  {
    return {{false, false}, {true, true}, {true, false}};
  }
}

BOOST_DATA_TEST_CASE (read_only_is_stored, tribool_values(), read_only)
{
  BOOST_REQUIRE_EQUAL
    ( read_only
    , xml::parse::type::memory_buffer_type
        ( xml::parse::util::position_type
          (nullptr, nullptr, fhg::util::testing::random_string())
        , fhg::util::testing::random_string()
        , fhg::util::testing::random_string()
        , fhg::util::testing::random_string()
        , read_only
        , we::type::property::type()
        ).read_only()
      );
}

BOOST_AUTO_TEST_CASE (name_is_unique_key)
{
  std::string const name (fhg::util::testing::random_string());

  BOOST_REQUIRE_EQUAL
    ( name
    , xml::parse::type::memory_buffer_type
      ( xml::parse::util::position_type
        (nullptr, nullptr, fhg::util::testing::random_string())
      , name
      , fhg::util::testing::random_string()
      , fhg::util::testing::random_string()
      , ::boost::none
      , we::type::property::type()
      ).unique_key()
    );
}

BOOST_DATA_TEST_CASE (dump, tribool_values(), read_only)
{
  std::string const name (fhg::util::testing::random_identifier());
  std::string const size (fhg::util::testing::random_string_without_zero());
  std::string const alignment
    (fhg::util::testing::random_string_without_zero());

  xml::parse::type::memory_buffer_type mb
    ( xml::parse::util::position_type
        (nullptr, nullptr, fhg::util::testing::random_string())
    , name
    , size
    , alignment
    , read_only
    , we::type::property::type()
    );

  std::ostringstream oss;

  fhg::util::xml::xmlstream s (oss);

  xml::parse::type::dump::dump (s, mb);

  const std::string expected
    ( fmt::format (R"EOS(<memory-buffer name="{0}"{2}>
  <size>{1}</size>
  <alignment>{3}</alignment>
</memory-buffer>)EOS"
      , name
      , size
      , ( !::boost::get_pointer (read_only) ? std::string()
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

  fhg::util::temporary_path const scoped_gen_output_path;
  ::boost::filesystem::path const gen_output_path
    (scoped_gen_output_path);

  xml::parse::state::type state;
  state.path_to_cpp() = gen_output_path.string();

  xml::parse::generate_cpp
    (xml::parse::just_parse (state, input_stream), state);

  BOOST_REQUIRE_EQUAL
    ( fhg::util::read_file<std::string>
        (gen_output_path / "pnetc" / "op" / "m" / "f.cpp")
    , expected_generated_cpp
    );
}
