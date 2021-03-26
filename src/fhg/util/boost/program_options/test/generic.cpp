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

#include <fhg/util/boost/program_options/generic.hpp>

#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/format.hpp>
#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>

namespace
{
  std::vector<std::string> options (std::initializer_list<std::string> options)
  {
    return options;
  }

  template<typename E>
    using exception = boost::exception_detail::clone_impl
                        <boost::exception_detail::error_info_injector<E>>;
}

BOOST_AUTO_TEST_CASE (option)
{
  namespace po = fhg::util::boost::program_options;

  std::string const name {fhg::util::testing::random_identifier()};
  std::string const description {fhg::util::testing::random_identifier()};

  po::option<std::string> const option {name, description};

  BOOST_REQUIRE_EQUAL (option.name(), name);
  BOOST_REQUIRE_EQUAL (option.description(), description);

  boost::program_options::variables_map vm;

  BOOST_REQUIRE (!option.is_contained_in (vm));

  fhg::util::testing::require_exception
    ( [&option, &vm] { option.get_from (vm); }
     , std::logic_error
       ((boost::format ("missing key '%1%' in variables map") % name).str())
    );

  std::string const value {fhg::util::testing::random_identifier()};

  BOOST_REQUIRE_EQUAL (option.get_from_or_value (vm, value), value);
  BOOST_REQUIRE (!option.get<std::string> (vm));

  option.set (vm, value);

  BOOST_REQUIRE (!option.defaulted (vm));
  BOOST_REQUIRE_EQUAL (option.get_from (vm), value);
  BOOST_REQUIRE_EQUAL (option.get_from_or_value (vm, {}), value);
  BOOST_REQUIRE_EQUAL (option.get<std::string> (vm).get(), value);

  std::string const other_value {fhg::util::testing::random_identifier()};

  fhg::util::testing::require_exception
    ( [&option, &vm, &other_value]
      {
        option.set (vm, other_value);
      }
    , std::logic_error
      ((boost::format ("Failed to set option '%1%' to '%2%': Found value '%3%'")
       % name
       % other_value
       % value
       ).str()
      )
    );
}

BOOST_AUTO_TEST_CASE (option_with_default)
{
  namespace po = fhg::util::boost::program_options;

  std::string const name {fhg::util::testing::random_identifier()};
  std::string const description {fhg::util::testing::random_identifier()};
  std::string const default_value {fhg::util::testing::random_identifier()};

  {
    po::option<std::string> const option {name, description, default_value};

    boost::program_options::variables_map vm
      (po::options ({}).add (option).store_and_notify (options ({})));

    BOOST_REQUIRE_EQUAL (option.get_from (vm), default_value);
    BOOST_REQUIRE (option.defaulted (vm));
  }

  {
    po::option<std::string> const option {name, description, default_value};

    std::string const value {fhg::util::testing::random_identifier()};

    boost::program_options::variables_map vm
      ( po::options ({})
      . add (option)
      . store_and_notify (options ({"--" + name, value}))
      );

    BOOST_REQUIRE_EQUAL (option.get_from (vm), value);
    BOOST_REQUIRE (!option.defaulted (vm));
  }
}

BOOST_AUTO_TEST_CASE (options_empty_options_is_okay)
{
  namespace po = fhg::util::boost::program_options;

  BOOST_REQUIRE (po::options ({}).store_and_notify (options ({})).empty());
}

BOOST_AUTO_TEST_CASE (options_no_options_are_added)
{
  namespace po = fhg::util::boost::program_options;

  std::string const s {fhg::util::testing::random_identifier()};

  BOOST_CHECK_EXCEPTION
    ( po::options ({}).store_and_notify (options ({s}));
    , exception<boost::program_options::too_many_positional_options_error>
    , [] ( exception<boost::program_options::too_many_positional_options_error>
             const& e
         )
      {
        return std::string (e.what()) == "too many positional options have"
                                        " been specified on the command line";
      }
    );

  BOOST_CHECK_EXCEPTION
    ( po::options ({}).store_and_notify (options ({"--" + s}));
    , exception<boost::program_options::unknown_option>
    , [&s] (exception<boost::program_options::unknown_option> const& e)
      {
        return std::string (e.what()) ==
          (boost::format ("unrecognised option '--%1%'") % s).str();
      }
    );
}

BOOST_AUTO_TEST_CASE (options_help_is_added)
{
  namespace po = fhg::util::boost::program_options;

  fhg::util::testing::require_exception
    ( [] { po::options ({}).store_and_notify (options ({"--help"})); }
    , std::runtime_error (R"EOS(
Common:
  -h [ --help ]          show the options description
)EOS")
    );

  fhg::util::testing::require_exception
    ( [] { po::options ({}).store_and_notify (options ({"-h"})); }
    , std::runtime_error (R"EOS(
Common:
  -h [ --help ]          show the options description
)EOS")
    );
}

BOOST_AUTO_TEST_CASE (options_header_is_stored)
{
  namespace po = fhg::util::boost::program_options;

  std::string const header {fhg::util::testing::random_identifier()};

  fhg::util::testing::require_exception
    ( [&header] { po::options (header).store_and_notify (options ({"-h"})); }
    , std::runtime_error ( ( boost::format (R"EOS(%1%:

Common:
  -h [ --help ]          show the options description
)EOS")
                           % header
                           ).str()
                         )
    );
}

BOOST_AUTO_TEST_CASE (options_with_version)
{
  namespace po = fhg::util::boost::program_options;

  std::string const v {fhg::util::testing::random_identifier()};

  fhg::util::testing::require_exception
    ( [&v] { po::options ({}).version (v).store_and_notify (options ({"-h"})); }
    , std::runtime_error (R"EOS(
Common:
  -h [ --help ]          show the options description
  -v [ --version ]       show the program version
)EOS")
    );

  fhg::util::testing::require_exception
    ( [&v] { po::options ({}).version (v).store_and_notify (options ({"-v"})); }
    , std::runtime_error (v)
    );

  fhg::util::testing::require_exception
    ( [&v]
      {
        po::options ({}).version (v).store_and_notify (options ({"--version"}));
      }
    , std::runtime_error (v)
    );
}

BOOST_AUTO_TEST_CASE (version_can_be_repeated)
{
  namespace po = fhg::util::boost::program_options;

  std::string const v {fhg::util::testing::random_identifier()};

  fhg::util::testing::require_exception
    ( [&v] { po::options ({})
           . version (v)
           . version (v)
           . store_and_notify (options ({"-v"}))
           ;
           }
    , std::runtime_error (v)
    );

  fhg::util::testing::require_exception
    ( [&v] { po::options ({})
           . version (v)
           . add (po::options ({}).version (v))
           . store_and_notify (options ({"-v"}))
           ;
           }
    , std::runtime_error (v)
    );
}

BOOST_AUTO_TEST_CASE (options_repeated_version_must_be_unique)
{
  namespace po = fhg::util::boost::program_options;

  std::string const v1 {fhg::util::testing::random_identifier()};
  std::string const v2 {fhg::util::testing::random_identifier()};

  fhg::util::testing::require_exception
    ( [&v1, &v2] { po::options ({}).version (v1).version (v2); }
    , std::invalid_argument
      ((boost::format ("Different versions: '%1%' vs '%2%'") % v1 % v2).str())
    );

  fhg::util::testing::require_exception
    ( [&v1, &v2]
      { po::options ({}).version (v1).add (po::options ({}).version (v2)); }
    , std::invalid_argument
      ((boost::format ("Different versions: '%1%' vs '%2%'") % v1 % v2).str())
    );
}

BOOST_AUTO_TEST_CASE (options_help_is_more_important_than_version)
{
  namespace po = fhg::util::boost::program_options;

  std::string const v {fhg::util::testing::random_identifier()};

  fhg::util::testing::require_exception
    ( [&v]
      {
        po::options ({}).version (v).store_and_notify (options ({"-h", "-v"}));
      }
    , std::runtime_error (R"EOS(
Common:
  -h [ --help ]          show the options description
  -v [ --version ]       show the program version
)EOS")
    );

  fhg::util::testing::require_exception
    ( [&v]
      {
        po::options ({}).version (v).store_and_notify (options ({"-v", "-h"}));
      }
    , std::runtime_error (R"EOS(
Common:
  -h [ --help ]          show the options description
  -v [ --version ]       show the program version
)EOS")
    );
}

BOOST_AUTO_TEST_CASE (options_add)
{
  namespace po = fhg::util::boost::program_options;

  std::string const name {fhg::util::testing::random_identifier()};
  std::string const description {fhg::util::testing::random_identifier()};

  {
    po::option<std::string> const option {name, description};

    BOOST_REQUIRE
      ( !option.is_contained_in ( po::options ({})
                                . add (option)
                                . store_and_notify (options ({}))
                                )
      );
  }

  std::string const value {fhg::util::testing::random_identifier()};

  {
    po::option<std::string> const option {name, description};

    BOOST_REQUIRE_EQUAL
      ( option.get_from ( po::options ({})
                        . add (option)
                        . store_and_notify (options ({"--" + name, value}))
                        )
      , value
      );
  }
}

BOOST_AUTO_TEST_CASE (options_require)
{
  namespace po = fhg::util::boost::program_options;

  std::string const name {fhg::util::testing::random_identifier()};
  std::string const description {fhg::util::testing::random_identifier()};

  {
    po::option<std::string> const option {name, description};

    BOOST_CHECK_EXCEPTION
      ( po::options ({}).require (option).store_and_notify (options ({}));
      , exception<boost::program_options::required_option>
      , [&name] (exception<boost::program_options::required_option> const& e)
        {
          return std::string (e.what()) ==
            "the option '--" + name + "' is required but missing";
        }
      );
  }

  std::string const value {fhg::util::testing::random_identifier()};

  {
    po::option<std::string> const option {name, description};

    BOOST_REQUIRE_EQUAL
      ( option.get_from ( po::options ({})
                        . require (option)
                        . store_and_notify (options ({"--" + name, value}))
                        )
      , value
      );
  }
}

BOOST_AUTO_TEST_CASE (options_require_defaulted)
{
  namespace po = fhg::util::boost::program_options;

  std::string const name {fhg::util::testing::random_identifier()};
  std::string const description {fhg::util::testing::random_identifier()};
  std::string const default_value {fhg::util::testing::random_identifier()};

  po::option<std::string> const option {name, description, default_value};

  boost::program_options::variables_map const vm
    (po::options ({}).require (option).store_and_notify (options ({})));

  BOOST_REQUIRE (option.is_contained_in (vm));
  BOOST_REQUIRE_EQUAL (option.get_from (vm), default_value);
  BOOST_REQUIRE (option.defaulted (vm));
}

BOOST_AUTO_TEST_CASE (options_positional)
{
  namespace po = fhg::util::boost::program_options;

  std::string const name {fhg::util::testing::random_identifier()};
  std::string const description {fhg::util::testing::random_identifier()};

  po::option<std::string> const option {name, description};

  std::string const value {fhg::util::testing::random_identifier()};

  BOOST_REQUIRE_EQUAL
    ( option.get_from ( po::options ({})
                      . add (option)
                      . positional (option)
                      . store_and_notify (options ({value}))
                      )
    , value
    );
}

BOOST_AUTO_TEST_CASE (options_only_single_positional_is_supported)
{
  namespace po = fhg::util::boost::program_options;

  std::string const name1 {fhg::util::testing::random_identifier()};
  std::string const description1 {fhg::util::testing::random_identifier()};
  std::string const name2 {fhg::util::testing::random_identifier()};
  std::string const description2 {fhg::util::testing::random_identifier()};

  {
    po::option<std::string> const option1 {name1, description1};
    po::option<std::string> const option2 {name2, description2};

    fhg::util::testing::require_exception
      ( [&] { po::options ({})
            . add (option1)
            . add (option2)
            . positional (option1)
            . positional (option2)
            ;
            }
      , std::logic_error ("more than one positional parameter")
      );
  }

  {
    po::option<std::string> const option1 {name1, description1};
    po::option<std::string> const option2 {name2, description2};

    fhg::util::testing::require_exception
      ( [&] { po::options ({})
            . add (option1)
            . positional (option1)
            . add ( po::options ({})
                  . add (option2)
                  . positional (option2)
                  )
            ;
            }
      , std::logic_error ("more than one positional parameter")
      );
  }
}

BOOST_AUTO_TEST_CASE (options_embedded)
{
  namespace po = fhg::util::boost::program_options;

  std::string const name {fhg::util::testing::random_identifier()};
  std::string const description {fhg::util::testing::random_identifier()};
  std::string const marker_on {fhg::util::testing::random_identifier()};
  std::string const marker_off {fhg::util::testing::random_identifier()};

  {
    po::embedded_command_line const embedded
      {name, description, marker_on, marker_off};

    BOOST_REQUIRE
      (embedded.get_from ( po::options ({})
                         . embed (embedded)
                         . store_and_notify (options ({}))
                         ).empty()
      );
  }

  {
    po::embedded_command_line const embedded
      {name, description, marker_on, marker_off};

    BOOST_REQUIRE (embedded.get_from
                    ( po::options ({})
                    . embed (embedded)
                    . store_and_notify (options ({marker_on, marker_off}))
                    ).empty()
                  );
  }

  {
    po::embedded_command_line const embedded
      {name, description, marker_on, marker_off};

    std::string const value {fhg::util::testing::random_identifier()};

    std::vector<std::string> const options_for_embedded
      ( embedded.get_from
          ( po::options ({})
          . embed (embedded)
          . store_and_notify (options ({marker_on, value, marker_off}))
          )
      );

    BOOST_REQUIRE_EQUAL (options_for_embedded.size(), 1);
    BOOST_REQUIRE_EQUAL (options_for_embedded[0], value);
  }

  {
    po::embedded_command_line const embedded
      {name, description, marker_on, marker_off};

    std::string const v1 {fhg::util::testing::random_identifier()};
    std::string const v2 {fhg::util::testing::random_identifier()};

    std::vector<std::string> const options_for_embedded
      ( embedded.get_from
          ( po::options ({})
          . embed (embedded)
          . store_and_notify (options ({marker_on, v1, v2, marker_off}))
          )
      );

    BOOST_REQUIRE_EQUAL (options_for_embedded.size(), 2);
    BOOST_REQUIRE_EQUAL (options_for_embedded[0], v1);
    BOOST_REQUIRE_EQUAL (options_for_embedded[1], v2);
  }
}

BOOST_AUTO_TEST_CASE (options_embedded_must_have_unique_marker)
{
  namespace po = fhg::util::boost::program_options;

  std::string const name1 {fhg::util::testing::random_identifier()};
  std::string const description1 {fhg::util::testing::random_identifier()};
  std::string const marker_off1 {fhg::util::testing::random_identifier()};
  std::string const name2 {fhg::util::testing::random_identifier()};
  std::string const description2 {fhg::util::testing::random_identifier()};
  std::string const marker_off2 {fhg::util::testing::random_identifier()};
  std::string const marker_on {fhg::util::testing::random_identifier()};

  {
    po::embedded_command_line const embedded1
      {name1, description1, marker_on, marker_off1};
    po::embedded_command_line const embedded2
      {name2, description2, marker_on, marker_off2};

    fhg::util::testing::require_exception
      ( [&] { po::options ({}).embed (embedded1).embed (embedded2); }
      , std::invalid_argument
        ((boost::format ("Duplicate marker '%1%'") % marker_on).str())
      );
  }

  {
    po::embedded_command_line const embedded1
      {name1, description1, marker_on, marker_off1};
    po::embedded_command_line const embedded2
      {name2, description2, marker_on, marker_off2};

    fhg::util::testing::require_exception
      ( [&] { po::options ({})
            . embed (embedded1)
            . add ( po::options ({})
                  . embed (embedded2)
                  )
            ;
            }
      , std::invalid_argument
        ((boost::format ("Duplicate marker '%1%'") % marker_on).str())
      );
  }
}

BOOST_AUTO_TEST_CASE (options_embedded_in_help_message)
{
  namespace po = fhg::util::boost::program_options;

  std::string const name {"<name>"};
  std::string const description {"<description>"};
  std::string const marker_on {"<marker-on>"};
  std::string const marker_off {"<marker-off>"};

  po::embedded_command_line const embedded
    {name, description, marker_on, marker_off};

  fhg::util::testing::require_exception
    ( [&] { po::options ("<header>")
          . embed (embedded)
          . store_and_notify (options ({"-h"}))
          ;
          }
    , std::runtime_error (R"EOS(<header>:
  --<name> arg           <description>
                         Specify also as '<marker-on> options... <marker-off>'

Common:
  -h [ --help ]          show the options description
)EOS")
    );
}

BOOST_AUTO_TEST_CASE (options_compatible_with_standard_boost_options)
{
  namespace po = fhg::util::boost::program_options;

  std::string const section {fhg::util::testing::random_identifier()};
  std::string const name {fhg::util::testing::random_identifier()};
  std::string const description {fhg::util::testing::random_identifier()};

  ::boost::program_options::options_description options_descriptions (section);
  options_descriptions.add_options() (name.c_str(), description.c_str());

  boost::program_options::variables_map const vm
    ( po::options ({})
    . add (options_descriptions)
    . store_and_notify (options ({"--" + name}))
    );

  BOOST_REQUIRE_EQUAL (vm.count (name.c_str()), 1);
}

BOOST_AUTO_TEST_CASE (options_standard_boost_options_in_help_message)
{
  namespace po = fhg::util::boost::program_options;

  std::string const section {"<section>"};
  std::string const name {"<name>"};
  std::string const description {"<description>"};

  ::boost::program_options::options_description options_descriptions (section);
  options_descriptions.add_options() (name.c_str(), description.c_str());

  fhg::util::testing::require_exception
    ( [&] { po::options ("<header>")
          . add (options_descriptions)
          . store_and_notify (options ({"-h"}))
          ;
          }
    , std::runtime_error (R"EOS(<header>:

<section>:
  --<name>               <description>

Common:
  -h [ --help ]          show the options description
)EOS")
    );
}

BOOST_AUTO_TEST_CASE (guessing_is_disabled)
{
  namespace po = fhg::util::boost::program_options;

  std::string const name {fhg::util::testing::random_identifier()};
  std::string const value {fhg::util::testing::random_identifier()};

  for (std::size_t i {1}; i < name.size(); ++i)
  {
    po::option<std::string> const option (name, "");

    auto const s (name.substr (0, i));

    BOOST_CHECK_EXCEPTION
      ( po::options ({})
      . add (option)
      . store_and_notify (options ({"--" + s, value}))
      ;
      , exception<boost::program_options::unknown_option>
      , [&s] (exception<boost::program_options::unknown_option> const& e)
        {
          return std::string (e.what()) ==
            (boost::format ("unrecognised option '--%1%'") % s).str();
        }
      );
  }

  {
    po::option<std::string> const option (name, "");

    boost::program_options::variables_map const vm
      { po::options ({})
      . add (option)
      . store_and_notify (options ({"--" + name, value}))
      };

    BOOST_REQUIRE_EQUAL (option.get_from (vm), value);
  }
}
