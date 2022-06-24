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

#include <we/exception.hpp>

#include <we/type/value/poke.hpp>

#include <we/type/signature/boost/test/printer.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <we/signature_of.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/list.hpp>

namespace
{
  struct fix
  {
    std::list<std::string> path() const
    {
      std::list<std::string> path;
      path.push_back ("p");
      path.push_back ("p");

      return path;
    }

    pnet::type::value::value_type value() const
    {
      pnet::type::value::value_type v;
      pnet::type::value::poke ("e", v, 42UL);
      pnet::type::value::poke ("c", v, 'c');

      return v;
    }

    pnet::type::signature::signature_type signature() const
    {
      return pnet::signature_of (value());
    }
  };
}

BOOST_FIXTURE_TEST_CASE (type_mismatch, fix)
{
  const  pnet::exception::type_mismatch e (signature(), value(), path());

  BOOST_CHECK_EQUAL (e.signature(), signature());
  BOOST_CHECK_EQUAL (e.value(), value());
  BOOST_CHECK_EQUAL (e.path(), path());
  BOOST_CHECK_EQUAL (e.what(), std::string ("type error: type mismatch for field 'p.p': expected type 'struct :: [e :: unsigned long, c :: char]', value 'Struct [e := 42UL, c := 'c']' has type 'struct :: [e :: unsigned long, c :: char]'"));
}

BOOST_FIXTURE_TEST_CASE (missing_field, fix)
{
  const pnet::exception::missing_field e (signature(), value(), path());

  BOOST_CHECK_EQUAL (e.signature(), signature());
  BOOST_CHECK_EQUAL (e.path(), path());
  BOOST_CHECK_EQUAL (e.what(), std::string ("type error: missing field 'p.p' of type 'struct :: [e :: unsigned long, c :: char]' in value 'Struct [e := 42UL, c := 'c']'"));
}

BOOST_FIXTURE_TEST_CASE (unknown_field, fix)
{
  const pnet::exception::unknown_field e (value(), path());

  BOOST_CHECK_EQUAL (e.value(), value());
  BOOST_CHECK_EQUAL (e.path(), path());
  BOOST_CHECK_EQUAL (e.what(), std::string ("type error: unknown field 'p.p' with value 'Struct [e := 42UL, c := 'c']' of type 'struct :: [e :: unsigned long, c :: char]'"));
}
