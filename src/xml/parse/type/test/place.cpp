// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <xml/parse/type/place.hpp>
#include <xml/parse/util/position.hpp>

#include <fhg/util/xml.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random/string.hpp>

#include <fmt/core.h>
#include <sstream>

BOOST_AUTO_TEST_CASE (dump_no_virtual)
{
  std::string const name (fhg::util::testing::random_identifier());
  std::string const type (fhg::util::testing::random_identifier());

  std::ostringstream oss;
  fhg::util::xml::xmlstream os (oss);

  xml::parse::type::dump::dump
    ( os
    , xml::parse::type::place_type
      ( xml::parse::util::position_type
        (nullptr, nullptr, fhg::util::testing::random_string())
      , name
      , type
      , false
      , ::boost::none
      )
    );

  std::string const expected
    ( fmt::format ( R"EOS(<place name="{}" type="{}" virtual="false"/>)EOS"
                  , name
                  , type
                  )
    );

  BOOST_REQUIRE_EQUAL (oss.str(), expected);
}

BOOST_AUTO_TEST_CASE (dump_virtual)
{
  std::string const name (fhg::util::testing::random_identifier());
  std::string const type (fhg::util::testing::random_identifier());

  std::ostringstream oss;
  fhg::util::xml::xmlstream os (oss);

  xml::parse::type::dump::dump
    ( os
    , xml::parse::type::place_type
      ( xml::parse::util::position_type
        (nullptr, nullptr, fhg::util::testing::random_string())
      , name
      , type
      , true
      , ::boost::none
      )
    );

  std::string const expected
    ( fmt::format ( R"EOS(<place name="{}" type="{}" virtual="true"/>)EOS"
                  , name
                  , type
                  )
    );

  BOOST_REQUIRE_EQUAL (oss.str(), expected);
}

BOOST_AUTO_TEST_CASE (dump_token)
{
  std::string const name (fhg::util::testing::random_identifier());
  std::string const type (fhg::util::testing::random_identifier());

  std::ostringstream oss;
  fhg::util::xml::xmlstream os (oss);

  xml::parse::type::place_type place
    ( xml::parse::util::position_type
      (nullptr, nullptr, fhg::util::testing::random_string())
    , name
    , type
    , true
    , ::boost::none
    );

  std::string const token (fhg::util::testing::random_content_string());

  place.push_token (token);

  xml::parse::type::dump::dump (os, place);

  std::string const expected
    ( fmt::format ( R"EOS(<place name="{}" type="{}" virtual="true">
  <token>
    <value>{}</value>
  </token>
</place>)EOS"
                  , name
                  , type
                  , token
                  )
    );

  BOOST_REQUIRE_EQUAL (oss.str(), expected);
}

BOOST_AUTO_TEST_CASE (dump_tokens)
{
  std::string const name (fhg::util::testing::random_identifier());
  std::string const type (fhg::util::testing::random_identifier());

  std::ostringstream oss;
  fhg::util::xml::xmlstream os (oss);

  xml::parse::type::place_type place
    ( xml::parse::util::position_type
      (nullptr, nullptr, fhg::util::testing::random_string())
    , name
    , type
    , true
    , ::boost::none
    );

  std::list<std::string> const tokens
    {fhg::util::testing::random_content_string(), fhg::util::testing::random_content_string()};

  for (std::string const& token : tokens)
  {
    place.push_token (token);
  }

  xml::parse::type::dump::dump (os, place);

  BOOST_REQUIRE_EQUAL (tokens.size(), 2);

  std::string const expected
    ( fmt::format ( R"EOS(<place name="{}" type="{}" virtual="true">
  <token>
    <value>{}</value>
  </token>
  <token>
    <value>{}</value>
  </token>
</place>)EOS"
                  , name
                  , type
                  , *(tokens.begin())
                  , *(std::next (tokens.begin()))
                  )
    );

  BOOST_REQUIRE_EQUAL (oss.str(), expected);
}
