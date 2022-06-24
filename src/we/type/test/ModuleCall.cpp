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

#include <we/type/ModuleCall.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/random/string.hpp>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>

BOOST_AUTO_TEST_CASE (memory_buffer_sizes_no_buffers)
{
  we::type::ModuleCall const module_call
    ( fhg::util::testing::random_identifier()
    , fhg::util::testing::random_identifier()
    , std::unordered_map<std::string, we::type::MemoryBufferInfo>()
    , {}
    , {}
    , true
    , true
    );

  BOOST_REQUIRE_EQUAL (module_call.memory_buffer_size_total ({}), 0UL);
}

namespace
{
  template<typename Key, typename Value>
    bool subset ( std::unordered_map<Key, Value> const& lhs
                , std::unordered_map<Key, Value> const& rhs
                )
  {
    for (auto const& l : lhs)
    {
      if (!rhs.count (l.first))
      {
        return false;
      }

      if (rhs.at (l.first) != l.second)
      {
        return false;
      }
    }

    return true;
  }

  template<typename Key, typename Value>
    bool equal ( std::unordered_map<Key, Value> const& lhs
               , std::unordered_map<Key, Value> const& rhs
               )
  {
    return subset (lhs, rhs) && subset (rhs, lhs);
  }
}

BOOST_AUTO_TEST_CASE (memory_buffer_random_sizes_default_alignments)
{
  std::list<std::string> names
    {fhg::util::testing::random<unsigned long>()() % 100};

  std::generate ( names.begin(), names.end()
                , []{ return fhg::util::testing::random_identifier(); }
                );

  std::unordered_map<std::string, we::type::MemoryBufferInfo>
    memory_buffers;
  expr::eval::context context;
  unsigned long total (0);

  for (std::string const& name : names)
  {
    if (memory_buffers.emplace
          ( std::piecewise_construct
          , std::forward_as_tuple (name)
          , std::forward_as_tuple ( "${" + name + "}"
                                  , "1UL"
                                  )
          ).second
       )
    {
      unsigned long const value
        {fhg::util::testing::random<unsigned long>()()};
      context.bind_and_discard_ref ({name}, value);
      total += value;
    }
  }

  we::type::ModuleCall const module_call
    ( fhg::util::testing::random_identifier()
    , fhg::util::testing::random_identifier()
    , std::move (memory_buffers)
    , {}
    , {}
    , true
    , true
    );

  BOOST_REQUIRE_EQUAL (module_call.memory_buffer_size_total (context), total);
}

BOOST_AUTO_TEST_CASE (memory_buffer_random_sizes_and_alignments)
{
  std::list<std::string> names
    {fhg::util::testing::random<unsigned long>()() % 100};

  std::generate ( names.begin(), names.end()
                , []{ return fhg::util::testing::random_identifier(); }
                );

  std::unordered_map<std::string, we::type::MemoryBufferInfo>
    memory_buffers;
  std::unordered_map<std::string, unsigned long> expected;
  expr::eval::context context;

  for (std::string const& name : names)
  {
    std::string size_expr ("size_" + name);
    std::string alignment_expr ("alignment_" + name);

    if (memory_buffers.emplace
          ( std::piecewise_construct
          , std::forward_as_tuple (name)
          , std::forward_as_tuple ( "${" + size_expr + "}"
                                  , "${" + alignment_expr + "}"
                                  )
          ).second
       )
    {
      unsigned long const size
        (fhg::util::testing::random<unsigned long>{}(1000,0));
      context.bind_and_discard_ref ({size_expr}, size);
      expected.emplace (size_expr, size);

      unsigned long const alignment
        (1ul << fhg::util::testing::random<unsigned long>{}(10,0));
      context.bind_and_discard_ref ({alignment_expr}, alignment);
      expected.emplace (alignment_expr, alignment);
    }
  }

  we::type::ModuleCall const module_call
    ( fhg::util::testing::random_identifier()
    , fhg::util::testing::random_identifier()
    , std::move (memory_buffers)
    , {}
    , {}
    , true
    , true
    );

  for (auto const& name_and_buffer_info : module_call.memory_buffers())
  {
    BOOST_REQUIRE_EQUAL
      ( name_and_buffer_info.second.size (context)
      , expected.at ("size_" + name_and_buffer_info.first)
      );

    BOOST_REQUIRE_EQUAL
      ( name_and_buffer_info.second.alignment (context)
      , expected.at ("alignment_" + name_and_buffer_info.first)
      );
  }
}
