// Copyright (C) 2012-2016,2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <parser_fixture.hpp>
#include <gspc/xml/parse/error.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>

BOOST_FIXTURE_TEST_CASE (different_directions, parser_fixture)
{
  BOOST_REQUIRE_NO_THROW (parse ("connect_in_out_same.xpnet"));
  BOOST_REQUIRE_NO_THROW (parse ("connect_read_out_same.xpnet"));
}

BOOST_FIXTURE_TEST_CASE (in_and_read, parser_fixture)
{
  BOOST_REQUIRE_THROW ( parse ("connect_in_read_same.xpnet")
                      , ::gspc::xml::parse::error::duplicate_connect
                      );
}

BOOST_FIXTURE_TEST_CASE (one_direction_twice, parser_fixture)
{
  BOOST_REQUIRE_THROW ( parse ("connect_in_in_same.xpnet")
                      , ::gspc::xml::parse::error::duplicate_connect
                      );
  BOOST_REQUIRE_THROW ( parse ("connect_read_read_same.xpnet")
                      , ::gspc::xml::parse::error::duplicate_connect
                      );
  BOOST_REQUIRE_THROW ( parse ("connect_out_out_same.xpnet")
                      , ::gspc::xml::parse::error::duplicate_connect
                      );
}
