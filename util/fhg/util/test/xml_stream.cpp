// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE XMLStream
#include <boost/test/unit_test.hpp>

#include <fhg/util/xml.hpp>

#include <sstream>

BOOST_AUTO_TEST_CASE (assemble_and_output)
{
  std::stringstream output;
  fhg::util::xml::xmlstream s (output);

  s.open ("first");

  s.open ("second");

  s.attr ("key", "val");
  s.attr ("maybe_key", "Just val");
  s.attr ("maybe_key", boost::none);

  s.close ();

  s.open ("content");
  s.content (23);
  s.close();

  s.open ("more");
  s.open ("deeper");
  s.attr ("key", 23);
  s.close();
  s.close ();

  s.close();

  BOOST_REQUIRE_EQUAL ( output.str()
                      , "<first>\n"
                        "  <second key=\"val\" maybe_key=\"Just val\"/>\n"
                        "  <content>23</content>\n"
                        "  <more>\n"
                        "    <deeper key=\"23\"/>\n"
                        "  </more>\n"
                        "</first>"
                      );
}
