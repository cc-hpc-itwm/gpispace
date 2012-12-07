// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_XML_TESTS_PARSER_FIXTURE_HPP
#define FHG_XML_TESTS_PARSER_FIXTURE_HPP

#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
  struct parser_fixture
  {
    parser_fixture()
      : state()
    {
      BOOST_REQUIRE (boost::unit_test::framework::master_test_suite().argc >= 2);
      path_to_xpnets = boost::unit_test::framework::master_test_suite().argv[1];
    }

    ::xml::parse::id::ref::function parse (const boost::filesystem::path& path)
    {
      return ::xml::parse::just_parse (state, (path_to_xpnets / path).string());
    }

    ::xml::parse::state::type state;

    boost::filesystem::path path_to_xpnets;
  };
}

#endif
