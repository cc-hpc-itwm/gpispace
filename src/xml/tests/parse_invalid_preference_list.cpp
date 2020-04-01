#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>

#include <util-generic/testing/require_exception.hpp>

#include <xml/tests/parse_preference_list_with_modules_helpers.hpp>

namespace
{
  enum transition_type { net, module, expression, multi_module };

  std::vector<transition_type> const non_module_types
    { transition_type::net
    , transition_type::expression
    };

  std::string random_identifier_with_invalid_prefix()
  {
    return
      fhg::util::testing::random_char_of
      ("!@#$%^&*(){}:\"'/.,\\")
      +
      fhg::util::testing::random_string_of
      ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");
  }

  std::string create_non_preference_transition_data_type
    ( transition_type const type
    , boost::optional<std::string> const name = boost::none
    )
  {
    switch (type)
    {
      case transition_type::net:
      {
        return "<net></net>";
      }
      case transition_type::expression:
      {
        return "<expression></expression>";
      }
      case transition_type::module:
      {
        if (!name)
        {
          throw std::logic_error
            ("missing name to create module for pnet");
        }

        return "<module name=\"" + *name
               + "\" function=\"func()\">"
               + "</module>";
      }
      default:
      {
        throw std::logic_error
          ("invalid non-preference transition data_type specified");
      }
    }
  }

}

BOOST_AUTO_TEST_CASE (parse_preference_list_with_duplicates)
{
  std::string const name (fhg::util::testing::random_identifier());

  std::string const input
    ( ( boost::format (R"EOS(
        <defun>
          <preferences>
            <target>%1%</target>
            <target>%1%</target>
          </preferences>
        </defun>)EOS")
      % name
      ).str()
    );

  xml::parse::state::type state;

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::duplicate_preference>
      ( [&state, &input]
        {
          std::istringstream input_stream (input);
          xml::parse::just_parse (state, input_stream);
        }
      , boost::format ( "ERROR: duplicate target type '%1%' at %2%"
                        ", already in the preferences"
                      )
        % name
        % "[<stdin>:5:13]"
      );
}

BOOST_AUTO_TEST_CASE (parse_empty_preference_list)
{
  std::string const input
    ( ( boost::format (R"EOS(
        <defun>
          <preferences>
          </preferences>
        </defun>)EOS")
      ).str()
    );

  xml::parse::state::type state;

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::empty_preferences>
      ( [&state, &input]
        {
          std::istringstream input_stream (input);
          xml::parse::just_parse (state, input_stream);
        }
        , boost::format ( "ERROR: preferences enabled, but no targets"
                        " specified at %1%"
                      )
          % "[<stdin>:3:11]"
      );
}

BOOST_AUTO_TEST_CASE (parse_preference_list_with_invalid_identifier)
{
  std::string const name (random_identifier_with_invalid_prefix());

  std::string const input
    ( ( boost::format (R"EOS(
        <defun>
          <preferences>
            <target>%1%</target>
          </preferences>
        </defun>)EOS")
      % name
      ).str()
    );

  xml::parse::state::type state;

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::invalid_name>
      ( [&state, &input]
        {
          std::istringstream input_stream (input);
          xml::parse::just_parse (state, input_stream);
        }
      , boost::format ( "ERROR: %2% %1% is invalid"
                        " (not of the form: [a-zA-Z_][a-zA-Z_0-9]^*)"
                        " in %3%"
                      )
        % name
        % "target"
        % "\"<stdin>\""
      );
}

BOOST_DATA_TEST_CASE ( parse_preference_list_without_modules
                     , non_module_types
                     , type
                     )
{
  std::string const first_target_name
    ("first_" + fhg::util::testing::random_identifier());
  std::string const second_target_name
    ("second_" + fhg::util::testing::random_identifier());

  std::string const input
    ( ( boost::format (R"EOS(
        <defun>
          <preferences>
            <target>%1%</target>
            <target>%2%</target>
          </preferences>
          %3%
        </defun>)EOS")
      % first_target_name
      % second_target_name
      % create_non_preference_transition_data_type (type)
      ).str()
    );

  xml::parse::state::type state;

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::preferences_without_modules>
      ( [&state, &input]
        {
          std::istringstream input_stream (input);
          xml::parse::just_parse (state, input_stream);
        }
      , boost::format ( "ERROR: preferences enabled, but"
                        " no modules with target defined"
                        " in %1%"
                      )
        % "[<stdin>:2:9]"
     );
}

BOOST_AUTO_TEST_CASE (parse_multi_modules_with_a_module_without_target)
{
  std::string const mod_name_with_no_target
    (fhg::util::testing::random_identifier());
  std::list<xml::parse::type::preference_type> test_targets
    (gen_valid_targets (MAX_TARGETS));

  pnet_with_multi_modules const pnet_mixed_modules
    ( test_targets
    , test_targets
    , mod_name_with_no_target
    , create_non_preference_transition_data_type
      ( transition_type::module
      , mod_name_with_no_target
      )
    );

  xml::parse::state::type state;

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::missing_target_for_module>
      ( [&state, &pnet_mixed_modules]
        {
          std::istringstream input_stream
            (pnet_mixed_modules.pnet_xml);
          xml::parse::just_parse (state, input_stream);
        }
      , boost::format ( "ERROR: module '%1%' missing target"
                        " for multi-module transition at %2%"
                      )
                      % mod_name_with_no_target
                      % "[<stdin>:6:11]"
      );
}

BOOST_AUTO_TEST_CASE (parse_multi_modules_with_duplicate_module_targets)
{
  std::string const mod_name
    (fhg::util::testing::random_identifier());
  std::string const target_name_first
    (fhg::util::testing::random_identifier());
  std::string const target_name_second
    (fhg::util::testing::random_identifier());
  std::list<xml::parse::type::preference_type> test_targets
    ({target_name_first, target_name_first});

  pnet_with_multi_modules const pnet_dup_modules
    ( {target_name_first, target_name_second}
    , test_targets
    , mod_name
    );

  xml::parse::state::type state;

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::duplicate_module_for_target>
      ( [&state, &pnet_dup_modules]
        {
          std::istringstream input_stream
            (pnet_dup_modules.pnet_xml);
          xml::parse::just_parse (state, input_stream);
        }
      , boost::format ( "ERROR: duplicate module '%1%' for target '%2%'"
                        " at %3%"
                      )
                      % (mod_name + "_" + target_name_first)
                      % target_name_first
                      % "[<stdin>:8:1]"
      );
}

BOOST_AUTO_TEST_CASE (parse_multi_modules_without_preferences)
{
  std::string const mod_name_with_no_preferences
    (fhg::util::testing::random_identifier());
  std::string const target_name
    (fhg::util::testing::random_identifier());

  pnet_with_multi_modules const pnet_no_pref
    ( {}
    , {target_name}
    , mod_name_with_no_preferences
    );

  xml::parse::state::type state;

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::modules_without_preferences>
      ( [&state, &pnet_no_pref]
        {
          std::istringstream input_stream
            (pnet_no_pref.pnet_xml);
          xml::parse::just_parse (state, input_stream);
        }
      , boost::format ( "ERROR: module '%1%' defined with"
                        " target '%2%', but preferences"
                        " not enabled at %3%"
                      )
                    % mod_name_with_no_preferences
                    % target_name
                    % "[<stdin>:5:11]"
      );
}

//! \note the following test cases exist:
// +--------------------+----------------+---------------------------------------+
// | preference_targets | module_targets |                                       |
// +--------------------+----------------+---------------------------------------+
// | ∅                  | ∅              | good: default single module case      |
// | a                  | a              | good: multi-mod valid matching        |
// | ∅                  | a              | covered: modules_without_preferences  |
// | a                  | ∅              | covered: preferences_without_modules  |
// |                    |                |          & missing_target_for_module  |
// | a                  | c              | left: [a], [c]                        |
// | a, b               | b, c           | left: [a], [c], match: [b]            |
// | a                  | a, b           | left: [], [b], match: [a]             |
// | a, b               | b              | left: [a], [], match: [b]             |
// +--------------------+----------------+---------------------------------------+
//
// "parse_multi_modules_with_mismatching_preferences" covers the last four cases,
// involving non-empty preferences and modules with target.
// the below constraints are posed to trigger the one exception type corresponding
// to them (i.e., error::mismatching_modules_and_preferences):
// * Roll a unique<list<string>> (count >= 2) and sorting that (the exception diff
// is always sorted)
// * Roll iterators: it_a = next (begin, [1, count-1]), it_b = next (begin, [0, a])
// [begin, a) and [b, end) ranges generate preferences and module targets, resp.
// [begin, b) and [a, end) ranges generate diff for preferences and modules, resp.
namespace
{
  constexpr auto const MIN_TARGETS = 2;

  size_t roll_random_number ( size_t min_inclusive
                            , size_t max_inclusive
                            )
  {
    return ( fhg::util::testing::random<size_t>{}()
             % (max_inclusive - min_inclusive + 1)
           )
           + min_inclusive;
  }

  struct mismatching_targets
  {
    std::list<xml::parse::type::preference_type> preferences;
    std::list<xml::parse::type::preference_type> modules;
    std::list<xml::parse::type::preference_type> no_modules;
    std::list<xml::parse::type::preference_type> no_preferences;

    mismatching_targets()
    {
      std::list<xml::parse::type::preference_type> targets;
      do
      {
        targets = gen_valid_targets (MAX_TARGETS);
      } while (targets.size() < MIN_TARGETS);

      auto const split_a =
        roll_random_number ( 1
                           , targets.size() - 1
                           );

      auto const split_b =
        roll_random_number ( 0
                           , split_a
                           );

      auto it_a = std::next ( targets.begin()
                            , split_a
                            );

      auto it_b = std::next ( targets.begin()
                            , split_b
                            );

      preferences = std::list<xml::parse::type::preference_type>
                    ( targets.begin()
                    , it_a
                    );

      modules = std::list<xml::parse::type::preference_type>
                ( it_b
                , targets.end()
                );

      no_modules = std::list<xml::parse::type::preference_type>
                   ( targets.begin()
                   , it_b
                   );

      no_preferences = std::list<xml::parse::type::preference_type>
                  ( it_a
                  , targets.end()
                  );
    }
  };
}

namespace
{
  template<typename T>
    std::list<T> sorted (std::list<T> list)
  {
    list.sort();
    return list;
  }
}

BOOST_AUTO_TEST_CASE (parse_multi_modules_with_non_empty_mismatching_preferences)
{
  mismatching_targets const targets;
  pnet_with_multi_modules const multi_mod ( targets.preferences
                                          , targets.modules
                                          );

  xml::parse::state::type state;

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::mismatching_modules_and_preferences>
      ( [&state, &multi_mod]
        {
          std::istringstream input_stream
            (multi_mod.pnet_xml);
          xml::parse::just_parse (state, input_stream);
        }
      , boost::format ( "ERROR: mismatching targets for"
                        " multi-module transition in %3%, %1%%2%"
                      )
                      % ( targets.no_modules.empty()
                          ?  ""
                          : fhg::util::print_container
                            ( "mismatch-in-preferences ('"
                            , "', '"
                            , "')"
                            , sorted (targets.no_modules)
                            ).string()
                        )
                      % ( targets.no_preferences.empty()
                          ?  ""
                          : fhg::util::print_container
                            ( ", mismatch-in-modules ('"
                            , "', '"
                            , "')"
                            , sorted (targets.no_preferences)
                            ).string()
                        )
                      % "[<stdin>:2:9]"
      );
}
