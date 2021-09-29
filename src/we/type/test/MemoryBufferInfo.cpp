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

#include <we/type/MemoryBufferInfo.hpp>

#include <we/expr/eval/context.hpp>
#include <we/expr/type/Context.hpp>
#include <we/expr/type/testing/types.hpp>
#include <we/type/value/show.hpp>

#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE (throw_if_the_alignment_is_not_a_power_of_2)
{
  expr::eval::context context;

  unsigned long const size
    {fhg::util::testing::random<unsigned long>()()};
  auto const size_expr (fhg::util::testing::random_identifier());
  context.bind_and_discard_ref ({size_expr}, size);

  unsigned long const exp
    (fhg::util::testing::random<unsigned long>{} (10, 2));
  unsigned long const alignment
    { fhg::util::testing::random<unsigned long>{}
        ((1ul << exp) - 1, (1ul << (exp - 1)) + 1)
    };
  auto const alignment_expr (fhg::util::testing::random_identifier());
    context.bind_and_discard_ref ({alignment_expr}, alignment);

  we::type::MemoryBufferInfo MemoryBufferInfo
    ("${" + size_expr + "}", "${" + alignment_expr + "}");

  BOOST_REQUIRE_EQUAL (MemoryBufferInfo.size (context), size);
  BOOST_REQUIRE_EXCEPTION
    ( MemoryBufferInfo.alignment (context)
    , std::runtime_error
    , [&] (std::runtime_error const& exc)
      {
        return std::string (exc.what()).find
          ("Invalid alignment expression. "
           "The alignment should be a power of 2!"
          ) != std::string::npos;
      }
    );
}

BOOST_AUTO_TEST_CASE (size_and_alignment_as_expressions)
{
  expr::eval::context context;

  unsigned long const size
    {fhg::util::testing::random<unsigned long>()()};
  auto const size_expr (fhg::util::testing::random_identifier());
  context.bind_and_discard_ref ({size_expr}, size);

  unsigned long const exp
    {fhg::util::testing::random<unsigned long>{} (10, 0)};
  unsigned long const alignment (1ul << exp);
  auto const alignment_expr (fhg::util::testing::random_identifier());
    context.bind_and_discard_ref ({alignment_expr}, alignment);

  we::type::MemoryBufferInfo MemoryBufferInfo
    ("${" + size_expr + "}", "${" + alignment_expr + "}");

  BOOST_REQUIRE_EQUAL (MemoryBufferInfo.size (context), size);
  BOOST_REQUIRE_EQUAL (MemoryBufferInfo.alignment (context), alignment);
}

BOOST_AUTO_TEST_CASE (size_and_alignment_as_constants)
{
  expr::eval::context context;

  unsigned long const size
    {fhg::util::testing::random<unsigned long>()()};

  unsigned long const exp
    {fhg::util::testing::random<unsigned long>{} (10, 0)};
  unsigned long const alignment (1ul << exp);

  std::ostringstream sstr_size;
  sstr_size << pnet::type::value::show (size);
  std::ostringstream sstr_alignment;
  sstr_alignment << pnet::type::value::show (alignment);
  we::type::MemoryBufferInfo MemoryBufferInfo
    (sstr_size.str(), sstr_alignment.str());

  BOOST_REQUIRE_EQUAL (MemoryBufferInfo.size (context), size);
  BOOST_REQUIRE_EQUAL (MemoryBufferInfo.alignment (context), alignment);
}

BOOST_DATA_TEST_CASE
  ( size_expression_throws_when_not_of_type_ULong
  , expr::type::testing::all_types_except (expr::type::ULong())
  , td
  )
{
  we::type::MemoryBufferInfo const memory_buffer_info ("${size}", "${alignment}");
  expr::type::Context context;
  context.bind ({"size"}, td.type);
  fhg::util::testing::require_exception
    ( [&]
      {
        memory_buffer_info.assert_correct_expression_types (context);
      }
    , std::runtime_error ("In the <size> expression '${size}'")
    );
}

BOOST_DATA_TEST_CASE
  ( alignment_expression_throws_when_not_of_type_ULong
  , expr::type::testing::all_types_except (expr::type::ULong())
  , td
  )
{
  we::type::MemoryBufferInfo const memory_buffer_info ("${size}", "${alignment}");
  expr::type::Context context;
  context.bind ({"size"}, expr::type::ULong{});
  context.bind ({"alignment"}, td.type);
  fhg::util::testing::require_exception
    ( [&]
      {
        memory_buffer_info.assert_correct_expression_types (context);
      }
    , std::runtime_error ("In the <alignment> expression '${alignment}'")
    );
}

BOOST_AUTO_TEST_CASE
  (size_and_alignment_expression_of_type_ULong_are_accepted)
{
  we::type::MemoryBufferInfo const memory_buffer_info ("${size}", "${alignment}");
  expr::type::Context context;
  context.bind ({"size"}, expr::type::ULong{});
  context.bind ({"alignment"}, expr::type::ULong{});

  BOOST_REQUIRE_NO_THROW
    (memory_buffer_info.assert_correct_expression_types (context));
}
