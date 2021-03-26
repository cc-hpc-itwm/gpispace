// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <we/signature_of.hpp>

#include <we/type/value/poke.hpp>

#include <we/type/signature/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_AUTO_TEST_CASE (signature)
{
  using pnet::type::value::value_type;
  using pnet::type::value::poke;
  using pnet::type::signature::signature_type;
  using pnet::type::signature::structure_type;
  using pnet::type::signature::structured_type;
  using pnet::signature_of;

#define SIG(_s,_v...)                                                   \
  BOOST_CHECK_EQUAL (signature_type (_s), signature_of (value_type (_v)))
#define SIG_LITERAL(_s,_v...) SIG (std::string (_s), _v)

  SIG_LITERAL ("control", we::type::literal::control());
  SIG_LITERAL ("bool", true);
  SIG_LITERAL ("int", 23);
  SIG_LITERAL ("long", 23L);
  SIG_LITERAL ("unsigned int", 23U);
  SIG_LITERAL ("unsigned long", 23UL);
  SIG_LITERAL ("float", 0.0f);
  SIG_LITERAL ("double", 0.0);
  SIG_LITERAL ("char", 'c');
  SIG_LITERAL ("string", std::string (""));
  SIG_LITERAL ("bitset", bitsetofint::type());
  SIG_LITERAL ("bytearray", we::type::bytearray());
  SIG_LITERAL ("list", std::list<value_type>());
  SIG_LITERAL ("set", std::set<value_type>());
  SIG_LITERAL ("map", std::map<value_type, value_type>());

  {
    value_type p1;
    poke ("x", p1, 0.0f);
    poke ("y", p1, 1.0f);
    value_type p2;
    poke ("x", p2, 2.0f);
    poke ("y", p2, 3.0f);
    value_type l;
    poke ("p", l, p1);
    poke ("q", l, p2);

#define FIELD(_name,_type)                                      \
    std::make_pair (std::string (_name), std::string (_type))
#define STRUCT(_name,_type)                                     \
    structured_type (std::make_pair (std::string (_name), _type))

    structure_type point2D;
    point2D.push_back (FIELD ("x", "float"));
    point2D.push_back (FIELD ("y", "float"));
    structure_type line2D;
    line2D.push_back (STRUCT ("p", point2D));
    line2D.push_back (STRUCT ("q", point2D));

    SIG (STRUCT ("struct", line2D), l);

#undef STRUCT
#undef FIELD
 }

#undef SIG_LITERAL
#undef SIG
}
