// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <fhg/util/xml.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <sstream>

BOOST_AUTO_TEST_CASE (assemble_and_output)
{
  std::stringstream output;
  fhg::util::xml::xmlstream s (output);

  s.open ("first");

  s.open ("second");

  s.attr ("key", "val");
  s.attr ("maybe_key", "Just val");
  s.attr ("maybe_key", ::boost::none);

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
