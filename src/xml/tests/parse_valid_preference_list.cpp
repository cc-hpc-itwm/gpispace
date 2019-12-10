#include <boost/test/unit_test.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>

#include <we/type/activity.hpp>
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

  const std::list<xml::parse::type::preference_type>& out_targets
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

  we::type::activity_t activity
    (xml::parse::xml_to_we (fun, state));

  const std::list<xml::parse::type::preference_type>& out_targets
    = activity.preferences();

  BOOST_REQUIRE_EQUAL (multi_mod.test_targets, out_targets);
}

BOOST_AUTO_TEST_CASE (xml_parse_valid_preferences_and_generate_cpp)
{
  valid_pnet_with_multi_modules const multi_mod;

  xml::parse::state::type state;

  std::istringstream input_stream (multi_mod.pnet.pnet_xml);
  xml::parse::type::function_type fun
    = xml::parse::just_parse (state, input_stream);

  boost::filesystem::path const path
    ( fhg::util::executable_path().parent_path()
      / "gen_modules_with_preferences_test"
    );
  state.path_to_cpp() = path.string();

  BOOST_REQUIRE_NO_THROW (xml::parse::generate_cpp (fun, state));

  boost::filesystem::path const makefile_path
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

    boost::filesystem::path const wrapper_cpp
      (path.string() + "/pnetc/op/" + mod_name + ".cpp");

    boost::filesystem::path const func_def_cpp
      (path.string() + "/pnetc/op/" + mod_name + "/func.cpp");

    boost::filesystem::path const func_def_hpp
      (path.string() + "/pnetc/op/" + mod_name + "/func.hpp");

    //! \note check if files generated for target
    BOOST_TEST_CONTEXT ("cpp generation for target = " << target)
    {
      BOOST_REQUIRE (boost::filesystem::exists (wrapper_cpp));
      BOOST_REQUIRE (boost::filesystem::exists (func_def_cpp));
      BOOST_REQUIRE (boost::filesystem::exists (func_def_hpp));
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

    //! \note check makefile for wrapper register calls
    std::string const makefile
      {fhg::util::read_file<std::string> (wrapper_cpp)};
    BOOST_TEST_CONTEXT ("wrapper for target = " << target)
    {
      std::string wrapper_reg ( wrapper_prefix
                              + wrapper_func_reg
                              + mod_name
                              + wrapper_suffix
                              );
      BOOST_REQUIRE (  makefile.find (wrapper_reg)
                    != std::string::npos
                    );
    }
  }
}
