// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE xml_parse_type_place
#include <boost/test/unit_test.hpp>

#include <xml/parse/id/mapper.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/util/position.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/xml.hpp>
#include <util-generic/testing/random_string.hpp>

#include <boost/format.hpp>

#include <sstream>

BOOST_AUTO_TEST_CASE (dump_no_virtual)
{
  std::string const name (fhg::util::testing::random_identifier());
  std::string const type (fhg::util::testing::random_identifier());

  xml::parse::id::mapper mapper;

  std::ostringstream oss;
  fhg::util::xml::xmlstream os (oss);

  xml::parse::type::dump::dump
    ( os
    , xml::parse::type::place_type
      ( mapper.next_id()
      , &mapper
      , boost::none
      , xml::parse::util::position_type
        (nullptr, nullptr, fhg::util::testing::random_string())
      , name
      , type
      , false
      )
    );

  std::string const expected
    ( ( boost::format (R"EOS(<place name="%1%" type="%2%" virtual="false"/>)EOS")
      % name
      % type
      ).str()
    );

  BOOST_REQUIRE_EQUAL (oss.str(), expected);
}

BOOST_AUTO_TEST_CASE (dump_virtual)
{
  std::string const name (fhg::util::testing::random_identifier());
  std::string const type (fhg::util::testing::random_identifier());

  xml::parse::id::mapper mapper;

  std::ostringstream oss;
  fhg::util::xml::xmlstream os (oss);

  xml::parse::type::dump::dump
    ( os
    , xml::parse::type::place_type
      ( mapper.next_id()
      , &mapper
      , boost::none
      , xml::parse::util::position_type
        (nullptr, nullptr, fhg::util::testing::random_string())
      , name
      , type
      , true
      )
    );

  std::string const expected
    ( ( boost::format (R"EOS(<place name="%1%" type="%2%" virtual="true"/>)EOS")
      % name
      % type
      ).str()
    );

  BOOST_REQUIRE_EQUAL (oss.str(), expected);
}

BOOST_AUTO_TEST_CASE (dump_token)
{
  std::string const name (fhg::util::testing::random_identifier());
  std::string const type (fhg::util::testing::random_identifier());

  xml::parse::id::mapper mapper;

  std::ostringstream oss;
  fhg::util::xml::xmlstream os (oss);

  xml::parse::type::place_type place
    ( mapper.next_id()
    , &mapper
    , boost::none
    , xml::parse::util::position_type
      (nullptr, nullptr, fhg::util::testing::random_string())
    , name
    , type
    , true
    );

  std::string const token (fhg::util::testing::random_content_string());

  place.push_token (token);

  xml::parse::type::dump::dump (os, place);

  std::string const expected
    ( ( boost::format (R"EOS(<place name="%1%" type="%2%" virtual="true">
  <token>
    <value>%3%</value>
  </token>
</place>)EOS")
      % name
      % type
      % token
      ).str()
    );

  BOOST_REQUIRE_EQUAL (oss.str(), expected);
}

BOOST_AUTO_TEST_CASE (dump_tokens)
{
  std::string const name (fhg::util::testing::random_identifier());
  std::string const type (fhg::util::testing::random_identifier());

  xml::parse::id::mapper mapper;

  std::ostringstream oss;
  fhg::util::xml::xmlstream os (oss);

  xml::parse::type::place_type place
    ( mapper.next_id()
    , &mapper
    , boost::none
    , xml::parse::util::position_type
      (nullptr, nullptr, fhg::util::testing::random_string())
    , name
    , type
    , true
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
    ( ( boost::format (R"EOS(<place name="%1%" type="%2%" virtual="true">
  <token>
    <value>%3%</value>
  </token>
  <token>
    <value>%4%</value>
  </token>
</place>)EOS")
      % name
      % type
      % *(tokens.begin())
      % *(std::next (tokens.begin()))
      ).str()
    );

  BOOST_REQUIRE_EQUAL (oss.str(), expected);
}
