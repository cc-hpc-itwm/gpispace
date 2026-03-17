// Copyright (C) 2013-2016,2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/we/exception.hpp>

#include <gspc/we/type/value/poke.hpp>

#include <test/we/type/signature/boost/printer.hpp>
#include <gspc/testing/printer/we/type/value.hpp>

#include <gspc/we/signature_of.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/list.hpp>

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

    gspc::pnet::type::value::value_type value() const
    {
      gspc::pnet::type::value::value_type v;
      gspc::pnet::type::value::poke ("e", v, 42UL);
      gspc::pnet::type::value::poke ("c", v, 'c');

      return v;
    }

    gspc::pnet::type::signature::signature_type signature() const
    {
      return gspc::pnet::signature_of (value());
    }
  };
}

BOOST_FIXTURE_TEST_CASE (type_mismatch, fix)
{
  const  gspc::pnet::exception::type_mismatch e (signature(), value(), path());

  BOOST_CHECK_EQUAL (e.signature(), signature());
  BOOST_CHECK_EQUAL (e.value(), value());
  BOOST_CHECK_EQUAL (e.path(), path());
  BOOST_CHECK_EQUAL (e.what(), std::string ("type error: type mismatch for field 'p.p': expected type 'struct :: [e :: unsigned long, c :: char]', value 'Struct [e := 42UL, c := 'c']' has type 'struct :: [e :: unsigned long, c :: char]'"));
}

BOOST_FIXTURE_TEST_CASE (missing_field, fix)
{
  const gspc::pnet::exception::missing_field e (signature(), value(), path());

  BOOST_CHECK_EQUAL (e.signature(), signature());
  BOOST_CHECK_EQUAL (e.path(), path());
  BOOST_CHECK_EQUAL (e.what(), std::string ("type error: missing field 'p.p' of type 'struct :: [e :: unsigned long, c :: char]' in value 'Struct [e := 42UL, c := 'c']'"));
}

BOOST_FIXTURE_TEST_CASE (unknown_field, fix)
{
  const gspc::pnet::exception::unknown_field e (value(), path());

  BOOST_CHECK_EQUAL (e.value(), value());
  BOOST_CHECK_EQUAL (e.path(), path());
  BOOST_CHECK_EQUAL (e.what(), std::string ("type error: unknown field 'p.p' with value 'Struct [e := 42UL, c := 'c']' of type 'struct :: [e :: unsigned long, c :: char]'"));
}
