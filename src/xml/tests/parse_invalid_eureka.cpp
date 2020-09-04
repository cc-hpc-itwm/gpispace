// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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
#include <boost/test/data/test_case.hpp>

#include <parser_fixture.hpp>
#include <xml/parse/error.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <cstdio>
#include <sstream>
#include <string>

BOOST_FIXTURE_TEST_CASE ( duplicate_eureka_connections
                        , parser_fixture
                        )
{
  BOOST_REQUIRE_THROW
    ( parse ("connect_eureka_duplicate_port.xpnet")
    , ::xml::parse::error::duplicate_eureka
    );
}

BOOST_AUTO_TEST_CASE (no_output_port_for_eureka)
{
  std::string const trans_name
    (fhg::util::testing::random_identifier_without_leading_underscore());
  std::string const port_name
    (fhg::util::testing::random_identifier_without_leading_underscore());

  std::string const input
    ( ( boost::format (R"EOS(
<defun name="foo">
 <net>
  <transition name="%1%">
    <defun>
      <in name="%2%" type="set"/>
      <expression/>
    </defun>
    <connect-eureka port="%2%"/>
  </transition>
 </net>
</defun>)EOS")
      % trans_name
      % port_name
      ).str()
    );

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::connect_eureka_to_nonexistent_out_port>
    ( [&input]()
      { xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (xml::parse::just_parse (state, input_stream));
        xml::parse::post_processing_passes (function, &state);
      }
    , boost::format
        ( "ERROR: connect-eureka to non-existent output port %1%"
          " in transition %2% at %3%"
        )
    % port_name
    % trans_name
    % "[<stdin>:9:5]"
    );
}

BOOST_AUTO_TEST_CASE (output_port_for_eureka_type_mismatch)
{
  std::string const trans_name
    (fhg::util::testing::random_identifier_without_leading_underscore());
  std::string const port_name
    (fhg::util::testing::random_identifier_without_leading_underscore());

  //! \note non-set data type
  std::string const type_name ("list");

  std::string const input
    ( ( boost::format (R"EOS(
<defun name="foo">
 <net>
  <transition name="%1%">
    <defun>
      <out name="%2%" type="%3%"/>
      <expression/>
    </defun>
    <connect-eureka port="%2%"/>
  </transition>
 </net>
</defun>)EOS")
      % trans_name
      % port_name
      % type_name
      ).str()
    );

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::eureka_port_type_mismatch>
    ( [&input]()
      { xml::parse::state::type state;
        std::istringstream input_stream (input);
        auto function (xml::parse::just_parse (state, input_stream));
        xml::parse::post_processing_passes (function, &state);
      }
    , boost::format
        ( "ERROR: connect-eureka output port %1%"
          " is not of type \"set\""
          " in transition %2% at %3%"
        )
    % port_name
    % trans_name
    % "[<stdin>:9:5]"
    );
}

namespace
{
  struct random_identifier_without_leading_underscore
  {
    std::string operator()() const
    {
      return fhg::util::testing::random_identifier_without_leading_underscore();
    }
  };
  using unique_random_identifier_without_leading_underscore
    = fhg::util::testing::unique_random
        <std::string, random_identifier_without_leading_underscore>;
}

BOOST_FIXTURE_TEST_CASE ( mismatching_eureka_for_modules_with_target
                        , parser_fixture
                        )
{
  unique_random_identifier_without_leading_underscore target_names;
  unique_random_identifier_without_leading_underscore eureka_groups;

  std::string const mod_name
    (fhg::util::testing::random_identifier_without_leading_underscore());
  auto const target_a (target_names());
  auto const target_b (target_names());
  auto const eureka_a (eureka_groups());
  auto const eureka_b (eureka_groups());

  std::string const input
    ( ( boost::format (R"EOS(
<defun name="foo">
 <net>
  <transition name="foo">
    <defun>
      <preferences>
        <target>%2%</target>
        <target>%3%</target>
      </preferences>
      <module name="%1%" function="func()" target="%2%" eureka-group="%4%"/>
      <module name="%1%" function="func()" target="%3%" eureka-group="%5%"/>
    </defun>
  </transition>
 </net>
</defun>)EOS")
      % mod_name
      % target_a
      % target_b
      % eureka_a
      % eureka_b
      ).str()
    );

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::mismatching_eureka_for_module>
    ( [&input]()
      { xml::parse::state::type state;
        std::istringstream input_stream (input);
        xml::parse::just_parse (state, input_stream);
      }
    , boost::format
        ( "ERROR: mismatching eureka group for module '%1%'"
          " in multi-module transition at %2%"
        )
    % (mod_name + "_" + target_a)
    % "[<stdin>:5:5]"
    );
}

BOOST_DATA_TEST_CASE ( missing_eureka_for_modules_with_target
                     , std::vector<bool> ({true, false})
                     , eureka_missing_on_first
                     )
{
  unique_random_identifier_without_leading_underscore target_names;

  std::string const mod_name
    (fhg::util::testing::random_identifier_without_leading_underscore());
  auto const target_a (target_names());
  auto const target_b (target_names());
  std::string const eureka
    ( " eureka-group=\""
    + fhg::util::testing::random_identifier_without_leading_underscore()
    + "\""
    );
  std::string const eureka_a
    (eureka_missing_on_first ? "" : eureka);
  std::string const eureka_b
    (eureka_missing_on_first ? eureka : "");

  std::string const input
    ( ( boost::format (R"EOS(
<defun name="foo">
 <net>
  <transition name="foo">
    <defun>
      <preferences>
        <target>%2%</target>
        <target>%3%</target>
      </preferences>
      <module name="%1%" function="func()" target="%2%"%4%/>
      <module name="%1%" function="func()" target="%3%"%5%/>
    </defun>
  </transition>
 </net>
</defun>)EOS")
      % mod_name
      % target_a
      % target_b
      % eureka_a
      % eureka_b
      ).str()
    );

  fhg::util::testing::require_exception_with_message
    <xml::parse::error::mismatching_eureka_for_module>
    ( [&input]()
      { xml::parse::state::type state;
        std::istringstream input_stream (input);
        xml::parse::just_parse (state, input_stream);
      }
    , boost::format
        ( "ERROR: mismatching eureka group for module '%1%'"
          " in multi-module transition at %2%"
        )
    % (mod_name + "_" + target_a)
    % "[<stdin>:5:5]"
    );
}
