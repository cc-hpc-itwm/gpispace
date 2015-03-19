// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE xml_parse_type_link
#include <boost/test/unit_test.hpp>

#include <xml/parse/type/link.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/xml.hpp>
#include <fhg/util/random_string.hpp>

#include <boost/format.hpp>

#include <map>
#include <stdexcept>
#include <sstream>

BOOST_AUTO_TEST_CASE (ctor_matches_member_observer)
{
  std::string const href (fhg::util::random_string());

  {
    xml::parse::type::link_type l (href, boost::none);

    BOOST_REQUIRE_EQUAL (l.href(), href);
    BOOST_REQUIRE (!l.prefix());
  }

  std::string const prefix (fhg::util::random_string());

  {
    xml::parse::type::link_type l (href, prefix);

    BOOST_REQUIRE_EQUAL (l.href(), href);
    BOOST_REQUIRE (l.prefix());
    BOOST_REQUIRE_EQUAL (l.prefix().get(), prefix);
  }
}

namespace
{
  std::string const& by_key_THROW (std::string const&)
  {
    throw std::runtime_error ("by_key_THROW called");
  }
}

BOOST_AUTO_TEST_CASE (link_no_prefix)
{
  std::string const href (fhg::util::random_string());

  BOOST_REQUIRE_EQUAL
    ( xml::parse::type::link_type (href, boost::none).link (&by_key_THROW)
    , href
    );
}

BOOST_AUTO_TEST_CASE (link_with_prefix_no_variable)
{
  std::string const href (fhg::util::random_string());
  std::string const prefix (fhg::util::random_string_without ("${}"));

  BOOST_REQUIRE_EQUAL
    ( xml::parse::type::link_type (href, prefix).link (&by_key_THROW)
    , prefix + "/" + href
    );
}

namespace
{
  std::string const& by_key_reverse (std::string const& key)
  {
    static std::map<std::string, std::string> m;

    return m.emplace (key, std::string (key.rbegin(), key.rend())).first->second;
  }
}

namespace
{
  std::string random_variable_name()
  {
    return fhg::util::random_string_without ("${}\\");
  }
}

BOOST_AUTO_TEST_CASE (link_with_prefix_is_variable)
{
  std::string const href (fhg::util::random_string());
  std::string const varname (random_variable_name());
  std::string const varname_reverse (varname.rbegin(), varname.rend());
  std::string const prefix ("${" + varname + "}");

  BOOST_REQUIRE_EQUAL
    ( xml::parse::type::link_type (href, prefix).link (&by_key_reverse)
    , varname_reverse + "/" + href
    );
}

BOOST_AUTO_TEST_CASE (link_with_prefix_many_variables)
{
  std::string const href (fhg::util::random_string());
  std::string const varname1 (random_variable_name());
  std::string const varname1_reverse (varname1.rbegin(), varname1.rend());
  std::string const varname2 (random_variable_name());
  std::string const varname2_reverse (varname2.rbegin(), varname2.rend());
  std::string const prefix ("${" + varname1 + "}/${" + varname2 + "}");

  BOOST_REQUIRE_EQUAL
    ( xml::parse::type::link_type (href, prefix).link (&by_key_reverse)
    , varname1_reverse + "/" + varname2_reverse + "/" + href
    );
}

BOOST_AUTO_TEST_CASE (dump_no_prefix)
{
  std::ostringstream oss;
  fhg::util::xml::xmlstream os (oss);

  std::string const href (fhg::util::random_string());

  xml::parse::type::dump::dump
    (os, xml::parse::type::link_type (href, boost::none));

  BOOST_REQUIRE_EQUAL
    (oss.str(), (boost::format ("<link href=\"%1%\"/>") % href).str());
}

BOOST_AUTO_TEST_CASE (dump_with_prefix)
{
  std::ostringstream oss;
  fhg::util::xml::xmlstream os (oss);

  std::string const href (fhg::util::random_string());
  std::string const prefix (fhg::util::random_string());

  xml::parse::type::dump::dump
    (os, xml::parse::type::link_type (href, prefix));

  BOOST_REQUIRE_EQUAL
    ( oss.str()
    , ( boost::format ("<link href=\"%1%\" prefix=\"%2%\"/>") % href % prefix
      ).str()
    );
}
