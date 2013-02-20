// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_XML_TESTS_PARSER_FIXTURE_HPP
#define FHG_XML_TESTS_PARSER_FIXTURE_HPP

#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>

#include <we/mgmt/type/activity.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
  struct parser_fixture
  {
    parser_fixture()
      : state()
      , function (boost::none)
    {
      BOOST_REQUIRE (boost::unit_test::framework::master_test_suite().argc >= 3);
      path_to_xpnets = boost::unit_test::framework::master_test_suite().argv[1];
      path_to_pnets = boost::unit_test::framework::master_test_suite().argv[2];
    }

    void parse (const boost::filesystem::path& path)
    {
      function = ::xml::parse::just_parse (state, (path_to_xpnets / path).string());
    }

    void post_processing_passes()
    {
      ::xml::parse::post_processing_passes (*function, &state);
    }

    void dump_xml()
    {
      ::xml::parse::dump_xml (*function, state);
    }

    ::we::mgmt::type::activity_t xml_to_we()
    {
      return ::xml::parse::xml_to_we (*function, state);
    }

    std::string content_of_file (const boost::filesystem::path& path) const
    {
      std::ifstream reference_file (path.string().c_str(), std::ios::in);

      BOOST_REQUIRE (reference_file);

      std::string reference;
      reference_file.seekg (0, std::ios::end);
      reference.resize (reference_file.tellg());
      reference_file.seekg (0, std::ios::beg);
      reference_file.read (&reference[0], reference.size());
      reference_file.close();

      return reference;
    }

    void require_same ( const ::we::mgmt::type::activity_t& activity
                      , const boost::filesystem::path& path
                      )
    {
      BOOST_REQUIRE_EQUAL ( content_of_file (path_to_pnets / path)
                          , activity.to_string()
                          );
    }

    ::xml::parse::state::type state;
    boost::optional< ::xml::parse::id::ref::function> function;

    boost::filesystem::path path_to_xpnets;
    boost::filesystem::path path_to_pnets;
  };
}

#endif
