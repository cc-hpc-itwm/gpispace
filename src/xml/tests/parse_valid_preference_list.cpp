// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <boost/test/unit_test.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>

#include <we/type/Activity.hpp>
#include <we/type/net.hpp>

#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/printer/list.hpp>

#include <util-generic/read_file.hpp>
#include <util-generic/executable_path.hpp>

#include <string>
#include <unordered_set>
#include <list>
#include <functional>

#include <boost/range/adaptors.hpp>

#include <xml/tests/parse_preference_list_with_modules_helpers.hpp>

namespace
{
  struct valid_pnet_with_multi_modules
  {
    std::list<std::string> const test_targets;
    pnet_with_multi_modules const pnet;

    valid_pnet_with_multi_modules ()
      : test_targets (gen_valid_targets (MAX_TARGETS))
      , pnet (test_targets, test_targets)
    {}
  };
}

BOOST_AUTO_TEST_CASE (xml_parse_valid_preferences_and_match_to_target_in_modules)
{
  valid_pnet_with_multi_modules const multi_mod;

  xml::parse::state::type state;
  std::istringstream input_stream (multi_mod.pnet.pnet_xml);

  BOOST_REQUIRE_NO_THROW (xml::parse::just_parse (state, input_stream));
}

BOOST_AUTO_TEST_CASE (xml_parse_valid_preferences_and_match_targets_in_function)
{
  valid_pnet_with_multi_modules const multi_mod;

  xml::parse::state::type state;

  std::istringstream input_stream (multi_mod.pnet.pnet_xml);
  xml::parse::type::function_type fun
    = xml::parse::just_parse (state, input_stream);

  std::list<xml::parse::type::preference_type> const& out_targets
    = fun.preferences().targets();

  BOOST_REQUIRE_EQUAL (multi_mod.test_targets, out_targets);
}

BOOST_AUTO_TEST_CASE (xml_parse_valid_preferences_and_propagate_to_we)
{
  valid_pnet_with_multi_modules const multi_mod;

  xml::parse::state::type state;

  std::istringstream input_stream (multi_mod.pnet.pnet_xml);
  xml::parse::type::function_type fun
    = xml::parse::just_parse (state, input_stream);

  auto const transition (xml::parse::xml_to_we (fun, state));

  std::list<xml::parse::type::preference_type> const& out_targets
    = transition.preferences();

  BOOST_REQUIRE_EQUAL (multi_mod.test_targets, out_targets);
}

BOOST_AUTO_TEST_CASE (xml_parse_valid_preferences_and_generate_cpp)
{
  valid_pnet_with_multi_modules const multi_mod;

  xml::parse::state::type state;

  std::istringstream input_stream (multi_mod.pnet.pnet_xml);
  xml::parse::type::function_type fun
    = xml::parse::just_parse (state, input_stream);

  ::boost::filesystem::path const path
    ( fhg::util::executable_path().parent_path()
      / "gen_modules_with_preferences_test"
    );
  state.path_to_cpp() = path.string();

  BOOST_REQUIRE_NO_THROW (xml::parse::generate_cpp (fun, state));

  ::boost::filesystem::path const makefile_path
    (path.string() + "/Makefile");
  std::string const makefile
    {fhg::util::read_file<std::string> (makefile_path)};

  std::string const wrapper_prefix
    ("WE_MOD_INITIALIZE_START()\n{\n");
  std::string const wrapper_func_reg
    ("  WE_REGISTER_FUN_AS (::pnetc::op::");
  std::string const wrapper_suffix
    ("::func,\"func\");\n}\nWE_MOD_INITIALIZE_END()");

  for (auto const& target : multi_mod.test_targets)
  {
    std::string const mod_name ( multi_mod.pnet.module_name
                               + "_" + target
                               );

    ::boost::filesystem::path const wrapper_cpp
      (path.string() + "/pnetc/op/" + mod_name + ".cpp");

    ::boost::filesystem::path const func_def_cpp
      (path.string() + "/pnetc/op/" + mod_name + "/func.cpp");

    ::boost::filesystem::path const func_def_hpp
      (path.string() + "/pnetc/op/" + mod_name + "/func.hpp");

    //! \note check if files generated for target
    BOOST_TEST_CONTEXT ("cpp generation for target = " << target)
    {
      BOOST_REQUIRE (::boost::filesystem::exists (wrapper_cpp));
      BOOST_REQUIRE (::boost::filesystem::exists (func_def_cpp));
      BOOST_REQUIRE (::boost::filesystem::exists (func_def_hpp));
    }

    //! \note check makefile for target ".so" build
    BOOST_TEST_CONTEXT ("lib for target = " << target)
    {
      std::string const lib_path
        ("pnetc/op/lib" + mod_name + ".so");
      BOOST_REQUIRE (  makefile.find ( "MODULES += "
                                     + lib_path
                                     )
                    != std::string::npos
                    );
    }

    //! \note check wrapper cpp, i.e., module_target.cpp, for
    //        wrapper register calls
    std::string const wrapper_cpp_content
      {fhg::util::read_file<std::string> (wrapper_cpp)};
    BOOST_TEST_CONTEXT ("wrapper for target = " << target)
    {
      std::string wrapper_reg ( wrapper_prefix
                              + wrapper_func_reg
                              + mod_name
                              + wrapper_suffix
                              );
      BOOST_REQUIRE (  wrapper_cpp_content.find (wrapper_reg)
                    != std::string::npos
                    );
    }
  }
}
