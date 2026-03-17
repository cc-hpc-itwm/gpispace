// Copyright (C) 2014-2016,2018,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/xml/parse/type/place.hpp>
#include <gspc/xml/parse/util/position.hpp>

#include <gspc/util/xml.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/random/string.hpp>

#include <fmt/core.h>
#include <sstream>
#include <optional>

BOOST_AUTO_TEST_CASE (dump_no_virtual)
{
  std::string const name (gspc::testing::random_identifier());
  std::string const type (gspc::testing::random_identifier());

  std::ostringstream oss;
  gspc::util::xml::xmlstream os (oss);

  gspc::xml::parse::type::dump::dump
    ( os
    , gspc::xml::parse::type::place_type
      ( gspc::xml::parse::util::position_type
        (nullptr, nullptr, gspc::testing::random_string())
      , name
      , type
      , false
      , std::nullopt
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
  std::string const name (gspc::testing::random_identifier());
  std::string const type (gspc::testing::random_identifier());

  std::ostringstream oss;
  gspc::util::xml::xmlstream os (oss);

  gspc::xml::parse::type::dump::dump
    ( os
    , gspc::xml::parse::type::place_type
      ( gspc::xml::parse::util::position_type
        (nullptr, nullptr, gspc::testing::random_string())
      , name
      , type
      , true
      , std::nullopt
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
  std::string const name (gspc::testing::random_identifier());
  std::string const type (gspc::testing::random_identifier());

  std::ostringstream oss;
  gspc::util::xml::xmlstream os (oss);

  gspc::xml::parse::type::place_type place
    ( gspc::xml::parse::util::position_type
      (nullptr, nullptr, gspc::testing::random_string())
    , name
    , type
    , true
    , std::nullopt
    );

  std::string const token (gspc::testing::random_content_string());

  place.push_token (token);

  gspc::xml::parse::type::dump::dump (os, place);

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
  std::string const name (gspc::testing::random_identifier());
  std::string const type (gspc::testing::random_identifier());

  std::ostringstream oss;
  gspc::util::xml::xmlstream os (oss);

  gspc::xml::parse::type::place_type place
    ( gspc::xml::parse::util::position_type
      (nullptr, nullptr, gspc::testing::random_string())
    , name
    , type
    , true
    , std::nullopt
    );

  std::list<std::string> const tokens
    {gspc::testing::random_content_string(), gspc::testing::random_content_string()};

  for (std::string const& token : tokens)
  {
    place.push_token (token);
  }

  gspc::xml::parse::type::dump::dump (os, place);

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
