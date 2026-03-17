// Copyright (C) 2011-2013,2015-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/util/xml.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>

#include <sstream>

BOOST_AUTO_TEST_CASE (assemble_and_output)
{
  std::stringstream output;
  gspc::util::xml::xmlstream s (output);

  s.open ("first");

  s.open ("second");

  s.attr ("key", "val");
  s.attr ("maybe_key", "Just val");
  s.attr ("maybe_key", std::nullopt);

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
