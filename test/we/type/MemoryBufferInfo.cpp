// Copyright (C) 2020-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/MemoryBufferInfo.hpp>

#include <gspc/we/expr/eval/context.hpp>
#include <gspc/we/expr/type/Context.hpp>
#include <gspc/we/expr/type/testing/types.hpp>
#include <gspc/we/type/value/show.hpp>

#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE (throw_if_the_alignment_is_not_a_power_of_2)
{
  gspc::we::expr::eval::context context;

  unsigned long const size
    {gspc::testing::random<unsigned long>()()};
  auto const size_expr (gspc::testing::random_identifier());
  context.bind_and_discard_ref ({size_expr}, size);

  unsigned long const exp
    (gspc::testing::random<unsigned long>{} (10, 2));
  unsigned long const alignment
    { gspc::testing::random<unsigned long>{}
        ((1ul << exp) - 1, (1ul << (exp - 1)) + 1)
    };
  auto const alignment_expr (gspc::testing::random_identifier());
    context.bind_and_discard_ref ({alignment_expr}, alignment);

  gspc::we::type::MemoryBufferInfo MemoryBufferInfo
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
  gspc::we::expr::eval::context context;

  unsigned long const size
    {gspc::testing::random<unsigned long>()()};
  auto const size_expr (gspc::testing::random_identifier());
  context.bind_and_discard_ref ({size_expr}, size);

  unsigned long const exp
    {gspc::testing::random<unsigned long>{} (10, 0)};
  unsigned long const alignment (1ul << exp);
  auto const alignment_expr (gspc::testing::random_identifier());
    context.bind_and_discard_ref ({alignment_expr}, alignment);

  gspc::we::type::MemoryBufferInfo MemoryBufferInfo
    ("${" + size_expr + "}", "${" + alignment_expr + "}");

  BOOST_REQUIRE_EQUAL (MemoryBufferInfo.size (context), size);
  BOOST_REQUIRE_EQUAL (MemoryBufferInfo.alignment (context), alignment);
}

BOOST_AUTO_TEST_CASE (size_and_alignment_as_constants)
{
  gspc::we::expr::eval::context context;

  unsigned long const size
    {gspc::testing::random<unsigned long>()()};

  unsigned long const exp
    {gspc::testing::random<unsigned long>{} (10, 0)};
  unsigned long const alignment (1ul << exp);

  std::ostringstream sstr_size;
  sstr_size << gspc::pnet::type::value::show (size);
  std::ostringstream sstr_alignment;
  sstr_alignment << gspc::pnet::type::value::show (alignment);
  gspc::we::type::MemoryBufferInfo MemoryBufferInfo
    (sstr_size.str(), sstr_alignment.str());

  BOOST_REQUIRE_EQUAL (MemoryBufferInfo.size (context), size);
  BOOST_REQUIRE_EQUAL (MemoryBufferInfo.alignment (context), alignment);
}

BOOST_DATA_TEST_CASE
  ( size_expression_throws_when_not_of_type_ULong
  , gspc::we::expr::type::testing::all_types_except (gspc::we::expr::type::ULong())
  , td
  )
{
  gspc::we::type::MemoryBufferInfo const memory_buffer_info ("${size}", "${alignment}");
  gspc::we::expr::type::Context context;
  context.bind ({"size"}, td.type);
  gspc::testing::require_exception
    ( [&]
      {
        memory_buffer_info.assert_correct_expression_types (context);
      }
    , std::runtime_error ("In the <size> expression '${size}'")
    );
}

BOOST_DATA_TEST_CASE
  ( alignment_expression_throws_when_not_of_type_ULong
  , gspc::we::expr::type::testing::all_types_except (gspc::we::expr::type::ULong())
  , td
  )
{
  gspc::we::type::MemoryBufferInfo const memory_buffer_info ("${size}", "${alignment}");
  gspc::we::expr::type::Context context;
  context.bind ({"size"}, gspc::we::expr::type::ULong{});
  context.bind ({"alignment"}, td.type);
  gspc::testing::require_exception
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
  gspc::we::type::MemoryBufferInfo const memory_buffer_info ("${size}", "${alignment}");
  gspc::we::expr::type::Context context;
  context.bind ({"size"}, gspc::we::expr::type::ULong{});
  context.bind ({"alignment"}, gspc::we::expr::type::ULong{});

  BOOST_REQUIRE_NO_THROW
    (memory_buffer_info.assert_correct_expression_types (context));
}
