// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <util-generic/boost/program_options/separated_argument_list_parser.hpp>

#include <boost/program_options.hpp>
#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>

namespace
{
  using options = std::vector<std::string>;
  constexpr char const* const name ("special");
  constexpr char const* const token_open ("+OPEN");
  constexpr char const* const token_close ("-CLOSE");
}

BOOST_AUTO_TEST_CASE (option_keeps_defaulted_when_unused)
{
  ::boost::program_options::options_description options_description;
  options_description.add_options()
    (name, ::boost::program_options::value<options>()->default_value (options(), ""));

  char const* argv[1] = {""};

  ::boost::program_options::variables_map vm;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser
        (sizeof (argv) / sizeof (decltype (argv[0])), argv)
    . options (options_description)
    . extra_style_parser
      ( fhg::util::boost::program_options::separated_argument_list_parser
          ({{token_open, {token_close, name}}})
      )
    . run()
    , vm
    );

  ::boost::program_options::notify (vm);

  BOOST_REQUIRE (vm.at (name).defaulted());
}

BOOST_AUTO_TEST_CASE (option_keeps_defaulted_when_nothing_follows_open)
{
  ::boost::program_options::options_description options_description;
  options_description.add_options()
    (name, ::boost::program_options::value<options>()->default_value (options(), ""));

  char const* argv[2] = {"", token_open};

  ::boost::program_options::variables_map vm;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser
        (sizeof (argv) / sizeof (decltype (argv[0])), argv)
    . options (options_description)
    . extra_style_parser
      ( fhg::util::boost::program_options::separated_argument_list_parser
          ({{token_open, {token_close, name}}})
      )
    . run()
    , vm
    );

  ::boost::program_options::notify (vm);

  BOOST_REQUIRE (vm.at (name).defaulted());
}

BOOST_AUTO_TEST_CASE (option_keeps_defaulted_when_nothing_in_between)
{
  ::boost::program_options::options_description options_description;
  options_description.add_options()
    (name, ::boost::program_options::value<options>()->default_value (options(), ""));

  char const* argv[3] = {"", token_open, token_close};

  ::boost::program_options::variables_map vm;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser
        (sizeof (argv) / sizeof (decltype (argv[0])), argv)
    . options (options_description)
    . extra_style_parser
      ( fhg::util::boost::program_options::separated_argument_list_parser
          ({{token_open, {token_close, name}}})
      )
    . run()
    , vm
    );

  ::boost::program_options::notify (vm);

  BOOST_REQUIRE (vm.at (name).defaulted());
}

BOOST_AUTO_TEST_CASE (option_is_rest_when_not_closed)
{
  ::boost::program_options::options_description options_description;
  options_description.add_options()
    (name, ::boost::program_options::value<options>()->default_value (options(), ""));

  char const* argv[4] = {"", token_open, "1", "2"};

  ::boost::program_options::variables_map vm;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser
        (sizeof (argv) / sizeof (decltype (argv[0])), argv)
    . options (options_description)
    . extra_style_parser
      ( fhg::util::boost::program_options::separated_argument_list_parser
          ({{token_open, {token_close, name}}})
      )
    . run()
    , vm
    );

  ::boost::program_options::notify (vm);

  options const got (vm.at (name).as<options>());

  BOOST_REQUIRE_EQUAL (got.size(), 2);
  BOOST_REQUIRE_EQUAL (got[0], "1");
  BOOST_REQUIRE_EQUAL (got[1], "2");
}

BOOST_AUTO_TEST_CASE (option_is_between_open_and_close)
{
  ::boost::program_options::options_description options_description;
  options_description.add_options()
    (name, ::boost::program_options::value<options>()->default_value (options(), ""));

  char const* argv[6] = {"", token_open, "1", "2", token_close, "3"};

  ::boost::program_options::variables_map vm;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser
        (sizeof (argv) / sizeof (decltype (argv[0])), argv)
    . options (options_description)
    . extra_style_parser
      ( fhg::util::boost::program_options::separated_argument_list_parser
          ({{token_open, {token_close, name}}})
      )
    . run()
    , vm
    );

  ::boost::program_options::notify (vm);

  options const got (vm.at (name).as<options>());

  BOOST_REQUIRE_EQUAL (got.size(), 2);
  BOOST_REQUIRE_EQUAL (got[0], "1");
  BOOST_REQUIRE_EQUAL (got[1], "2");
}

BOOST_AUTO_TEST_CASE (multiple_sections)
{
  ::boost::program_options::options_description options_description;
  options_description.add_options()
    ("1", ::boost::program_options::value<options>()->default_value (options(), ""))
    ("2", ::boost::program_options::value<options>()->default_value (options(), ""))
    ("p", ::boost::program_options::value<options>()->default_value (options(), ""))
    ;
  ::boost::program_options::positional_options_description positional;
  positional.add ("p", -1);

  char const* argv[10] = { ""
                         , "+1", "1", "-1"
                         , "p0"
                         , "+2", "21", "22", "-2"
                         , "p1"
                         };

  ::boost::program_options::variables_map vm;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser
        (sizeof (argv) / sizeof (decltype (argv[0])), argv)
    . options (options_description)
    . positional (positional)
    . extra_style_parser
      ( fhg::util::boost::program_options::separated_argument_list_parser
          ({{"+1", {"-1", "1"}}, {"+2", {"-2", "2"}}})
      )
    . run()
    , vm
    );

  ::boost::program_options::notify (vm);

  options const got1 (vm.at ("1").as<options>());
  options const got2 (vm.at ("2").as<options>());
  options const gotp (vm.at ("p").as<options>());

  BOOST_REQUIRE_EQUAL (got1.size(), 1);
  BOOST_REQUIRE_EQUAL (got2.size(), 2);
  BOOST_REQUIRE_EQUAL (gotp.size(), 2);
  BOOST_REQUIRE_EQUAL (got1[0], "1");
  BOOST_REQUIRE_EQUAL (got2[0], "21");
  BOOST_REQUIRE_EQUAL (got2[1], "22");
  BOOST_REQUIRE_EQUAL (gotp[0], "p0");
  BOOST_REQUIRE_EQUAL (gotp[1], "p1");
}

BOOST_AUTO_TEST_CASE (empty_description_does_not_affect_other_options)
{
  ::boost::program_options::options_description options_description;
  options_description.add_options()
    ("a", ::boost::program_options::value<options>()->default_value (options(), ""))
    ("b", ::boost::program_options::value<options>()->default_value (options(), ""))
    ;
  ::boost::program_options::positional_options_description positional;
  positional.add ("b", -1);

  char const* argv[8] = {"", "--a", "a", "--a", "a", "b", "b", "c"};

  ::boost::program_options::variables_map vm;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser
        (sizeof (argv) / sizeof (decltype (argv[0])), argv)
    . options (options_description)
    . positional (positional)
    . extra_style_parser
      ( fhg::util::boost::program_options::separated_argument_list_parser
          ({})
      )
    . run()
    , vm
    );

  ::boost::program_options::notify (vm);

  options const gota (vm.at ("a").as<options>());
  options const gotb (vm.at ("b").as<options>());

  BOOST_REQUIRE_EQUAL (gota.size(), 2);
  BOOST_REQUIRE_EQUAL (gota[0], "a");
  BOOST_REQUIRE_EQUAL (gota[1], "a");
  BOOST_REQUIRE_EQUAL (gotb.size(), 3);
  BOOST_REQUIRE_EQUAL (gotb[0], "b");
  BOOST_REQUIRE_EQUAL (gotb[1], "b");
  BOOST_REQUIRE_EQUAL (gotb[2], "c");
}

BOOST_AUTO_TEST_CASE (nesting_requires_second_pass)
{
  ::boost::program_options::options_description options_description;
  options_description.add_options()
    ("1", ::boost::program_options::value<options>()->default_value (options(), ""))
    ("2", ::boost::program_options::value<options>()->default_value (options(), ""))
    ;

  auto const style_parser
    ( fhg::util::boost::program_options::separated_argument_list_parser
        ({{"+1", {"-1", "1"}}, {"+2", {"-2", "2"}}})
    );

  auto const args ( options { "+1"
                              , "1"
                              , "+2"
                                , "21"
                                , "22"
                              , "-2"
                              , "2"
                            , "-1"
                            }
                  );

  ::boost::program_options::variables_map vm1;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser (args)
    . options (options_description)
    . extra_style_parser (style_parser)
    . run()
    , vm1
    );

  ::boost::program_options::notify (vm1);

  BOOST_TEST_REQUIRE ( (  vm1.at ("1").as<options>()
                       == options {"1", "+2", "21", "22", "-2", "2"}
                       )
                     );
  BOOST_TEST_CHECK (vm1.at ("2").as<options>().empty());

  ::boost::program_options::positional_options_description positional;
  positional.add ("1", -1);
  ::boost::program_options::variables_map vm2;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser (vm1.at ("1").as<options>())
    . options (options_description)
    . positional (positional)
    . extra_style_parser (style_parser)
    . run()
    , vm2
    );

  ::boost::program_options::notify (vm2);

  BOOST_TEST_CHECK ((vm2.at ("1").as<options>() == options {"1", "2"}));
  BOOST_TEST_CHECK ((vm2.at ("2").as<options>() == options {"21", "22"}));
}

BOOST_AUTO_TEST_CASE (markers_that_are_prefix_or_suffix_are_working)
{
  ::boost::program_options::options_description options_description;
  options_description.add_options()
    ("1", ::boost::program_options::value<options>()->default_value (options(), ""))
    ("2", ::boost::program_options::value<options>()->default_value (options(), ""))
    ;

  auto const args (options {"[", "1", "]]", "[[", "2", "]"});

  ::boost::program_options::variables_map vm;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser (args)
    . options (options_description)
    . extra_style_parser
      ( fhg::util::boost::program_options::separated_argument_list_parser
          ({{"[", {"]]", "1"}}, {"[[", {"]", "2"}}})
      )
    . run()
    , vm
    );

  ::boost::program_options::notify (vm);

  BOOST_TEST_REQUIRE ((vm.at ("1").as<options>() == options {"1"}));
  BOOST_TEST_REQUIRE ((vm.at ("2").as<options>() == options {"2"}));
}

BOOST_AUTO_TEST_CASE (multiple_section_occurences_are_working)
{
  ::boost::program_options::options_description options_description;
  options_description.add_options()
    ("1", ::boost::program_options::value<options>()->default_value (options(), ""))
    ("p", ::boost::program_options::value<options>()->default_value (options(), ""))
    ;
  ::boost::program_options::positional_options_description positional;
  positional.add ("p", -1);

  auto const args (options {"a", "[", "1", "]", "b", "[", "2", "]", "c"});

  ::boost::program_options::variables_map vm;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser (args)
    . options (options_description)
    . positional (positional)
    . extra_style_parser
      ( fhg::util::boost::program_options::separated_argument_list_parser
          ("[", "]", "1")
      )
    . run()
    , vm
    );

  ::boost::program_options::notify (vm);

  BOOST_TEST_REQUIRE ((vm.at ("1").as<options>() == options {"1", "2"}));
  BOOST_TEST_REQUIRE ((vm.at ("p").as<options>() == options {"a", "b", "c"}));
}

BOOST_AUTO_TEST_CASE (option_value_can_start_and_end_with_marker_values)
{
  ::boost::program_options::options_description options_description;
  options_description.add_options()
    ("1", ::boost::program_options::value<options>()->default_value (options(), ""))
    ;

  //! \note ']' does not work -> the section ends
  //! \note '[' does work -> is this wrong? is nesting meant?
  for (auto value : options {"[", "[]", "[v", "v]", "[v]"})
  {
    auto const args (options {"[", value, "]"});

    ::boost::program_options::variables_map vm;
    ::boost::program_options::store
      ( ::boost::program_options::command_line_parser (args)
      . options (options_description)
      . extra_style_parser
        ( fhg::util::boost::program_options::separated_argument_list_parser
            ("[", "]", "1")
        )
      . run()
      , vm
      );

    ::boost::program_options::notify (vm);

    BOOST_TEST_REQUIRE ((vm.at ("1").as<options>() == options {value}));
  }
}

BOOST_AUTO_TEST_CASE (markers_doesnt_require_whitespace_to_be_detected)
{
  ::boost::program_options::options_description options_description;
  options_description.add_options()
    ("1", ::boost::program_options::value<options>()->default_value (options(), ""))
    ("p", ::boost::program_options::value<options>()->default_value (options(), ""))
    ;
  ::boost::program_options::positional_options_description positional;
  positional.add ("p", -1);

  auto const style_parser
    ( fhg::util::boost::program_options::separated_argument_list_parser
        ("[", "]", "1")
    );

  for (auto args : { options {"[", "]"}
                   , options {"[]"}
                   }
    )
  {
    ::boost::program_options::variables_map vm;
    ::boost::program_options::store
      ( ::boost::program_options::command_line_parser (args)
      . options (options_description)
      . positional (positional)
      . extra_style_parser (style_parser)
      . run()
      , vm
      );

    ::boost::program_options::notify (vm);

    BOOST_TEST_REQUIRE (vm.at ("1").as<options>().empty());
    BOOST_TEST_REQUIRE (vm.at ("p").as<options>().empty());
  }

  for (auto args : { options {"[", "1", "]"}
                   , options {"[1", "]"}
                   , options {"[", "1]"}
                   , options {"[1]"}
                   }
    )
  {
    ::boost::program_options::variables_map vm;
    ::boost::program_options::store
      ( ::boost::program_options::command_line_parser (args)
      . options (options_description)
      . positional (positional)
      . extra_style_parser (style_parser)
      . run()
      , vm
      );

    ::boost::program_options::notify (vm);

    BOOST_TEST_REQUIRE ((vm.at ("1").as<options>() == options {"1"}));
    BOOST_TEST_REQUIRE (vm.at ("p").as<options>().empty());
  }

  for (auto args : { options {"a", "[1", "1]", "b"}
                   , options {"a", "[", "1", "1]", "b"}
                   , options {"a", "[1", "1", "]", "b"}
                   , options {"a", "[", "1", "1", "]", "b"}
                   }
      )
  {
    ::boost::program_options::variables_map vm;
    ::boost::program_options::store
      ( ::boost::program_options::command_line_parser (args)
      . options (options_description)
      . positional (positional)
      . extra_style_parser (style_parser)
      . run()
      , vm
      );

    ::boost::program_options::notify (vm);

    BOOST_TEST_REQUIRE ((vm.at ("1").as<options>() == options {"1", "1"}));
    BOOST_TEST_REQUIRE ((vm.at ("p").as<options>() == options {"a", "b"}));
  }
}

BOOST_AUTO_TEST_CASE (values_can_contain_marker_even_when_whitespace_is_omitted)
{
  ::boost::program_options::options_description options_description;
  options_description.add_options()
    ("1", ::boost::program_options::value<options>()->default_value (options(), ""))
    ("p", ::boost::program_options::value<options>()->default_value (options(), ""))
    ;
  ::boost::program_options::positional_options_description positional;
  positional.add ("p", -1);

  auto const style_parser
    ( fhg::util::boost::program_options::separated_argument_list_parser
        ("[", "]", "1")
    );

  // cases when the value contains markers, too
  // note that a full match has higher priority than a partial match

  // input       1       p
  // ---------- ------- ---
  // [ [1] ]     [1]
  // [[1] ]      [1]
  // [ [1]]      [1]
  // [[1]]       [1]

  // [[ 1]]      [ 1]
  // [ [ 1]]     [ 1]
  // [[ 1] ]     [ 1]
  // [ [ 1] ]    [ 1]

  // [[1 ]]      [1 ]
  // [ [1 ]]     [1 ]

  // [[1 ] ]     [1      ]

  // [[ 1 ]]     [ 1 ]
  // [ [ 1 ]]    [ 1 ]

  // [[ 1 ] ]    [ 1     ]
  // [ [ 1 ] ]   [ 1     ]

  for (auto args : { options {"[", "[1]", "]"}
                   , options {"[[1]", "]"}
                   , options {"[", "[1]]"}
                   , options {"[[1]]"}
                   }
    )
  {
    ::boost::program_options::variables_map vm;
    ::boost::program_options::store
      ( ::boost::program_options::command_line_parser (args)
      . options (options_description)
      . positional (positional)
      . extra_style_parser (style_parser)
      . run()
      , vm
      );

    ::boost::program_options::notify (vm);

    BOOST_TEST_REQUIRE ((vm.at ("1").as<options>() == options {"[1]"}));
    BOOST_TEST_REQUIRE (vm.at ("p").as<options>().empty());
  }

  for (auto args : { options {"[[", "1]]"}
                   , options {"[", "[", "1]]"}
                   , options {"[[", "1]", "]"}
                   , options {"[", "[", "1]", "]"}
                   }
    )
  {
    ::boost::program_options::variables_map vm;
    ::boost::program_options::store
      ( ::boost::program_options::command_line_parser (args)
      . options (options_description)
      . positional (positional)
      . extra_style_parser (style_parser)
      . run()
      , vm
      );

    ::boost::program_options::notify (vm);

    BOOST_TEST_REQUIRE ((vm.at ("1").as<options>() == options {"[", "1]"}));
    BOOST_TEST_REQUIRE (vm.at ("p").as<options>().empty());
  }

  for (auto args : { options {"[[1", "]]"}
                   , options {"[", "[1", "]]"}
                   }
    )
  {
    ::boost::program_options::variables_map vm;
    ::boost::program_options::store
      ( ::boost::program_options::command_line_parser (args)
      . options (options_description)
      . positional (positional)
      . extra_style_parser (style_parser)
      . run()
      , vm
      );

    ::boost::program_options::notify (vm);

    BOOST_TEST_REQUIRE ((vm.at ("1").as<options>() == options {"[1", "]"}));
    BOOST_TEST_REQUIRE (vm.at ("p").as<options>().empty());
  }

  for (auto args : { options {"[[1", "]", "]"}
                   }
      )
  {
    ::boost::program_options::variables_map vm;
    ::boost::program_options::store
      ( ::boost::program_options::command_line_parser (args)
      . options (options_description)
      . positional (positional)
      . extra_style_parser (style_parser)
      . run()
      , vm
      );

    ::boost::program_options::notify (vm);

    BOOST_TEST_REQUIRE ((vm.at ("1").as<options>() == options {"[1"}));
    BOOST_TEST_REQUIRE ((vm.at ("p").as<options>() == options {"]"}));
  }

  for (auto args : { options {"[[", "1", "]]"}
                   , options {"[", "[", "1", "]]"}
                   }
      )
  {
    ::boost::program_options::variables_map vm;
    ::boost::program_options::store
      ( ::boost::program_options::command_line_parser (args)
      . options (options_description)
      . positional (positional)
      . extra_style_parser (style_parser)
      . run()
      , vm
      );

    ::boost::program_options::notify (vm);

    BOOST_TEST_REQUIRE ((vm.at ("1").as<options>() == options {"[", "1", "]"}));
    BOOST_TEST_REQUIRE (vm.at ("p").as<options>().empty());
  }

  for (auto args : { options {"[[", "1", "]", "]"}
                   , options {"[", "[", "1", "]", "]"}
                   }
      )
  {
    ::boost::program_options::variables_map vm;
    ::boost::program_options::store
      ( ::boost::program_options::command_line_parser (args)
      . options (options_description)
      . positional (positional)
      . extra_style_parser (style_parser)
      . run()
      , vm
      );

    ::boost::program_options::notify (vm);

    BOOST_TEST_REQUIRE ((vm.at ("1").as<options>() == options {"[", "1"}));
    BOOST_TEST_REQUIRE ((vm.at ("p").as<options>() == options {"]"}));
  }
}
