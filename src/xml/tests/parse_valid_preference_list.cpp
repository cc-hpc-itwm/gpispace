#include <boost/test/unit_test.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>

#include <we/type/activity.hpp>
#include <we/type/net.hpp>

#include <util-generic/testing/random.hpp>
#include <util-generic/testing/random/string.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/printer/list.hpp>

#include <util-generic/join.hpp>

#include <string>
#include <unordered_set>
#include <list>
#include <functional>

#include <boost/range/adaptors.hpp>

#include <xml/tests/parse_preference_list_with_modules_helpers.ipp>

namespace
{
  struct valid_pnet_with_multi_modules
  {
    std::list<std::string> const test_targets;
    pnet_with_multi_modules const pnet;

    valid_pnet_with_multi_modules()
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
