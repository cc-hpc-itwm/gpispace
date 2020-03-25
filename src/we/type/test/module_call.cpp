#include <we/type/module_call.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random/string.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <list>
#include <string>
#include <unordered_map>

BOOST_AUTO_TEST_CASE (memory_buffer_sizes_no_buffers)
{
  we::type::module_call_t const module_call
    ( fhg::util::testing::random_identifier()
    , fhg::util::testing::random_identifier()
    , std::unordered_map<std::string, we::type::memory_buffer_info_t>()
    , {}
    , {}
    );

  BOOST_REQUIRE (module_call.memory_buffer_sizes ({}).empty());
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

BOOST_AUTO_TEST_CASE (memory_buffer_sizes)
{
  std::list<std::string> names
    {fhg::util::testing::random<unsigned long>()() % 100};

  std::generate ( names.begin(), names.end()
                , []{ return fhg::util::testing::random_identifier(); }
                );

  std::unordered_map<std::string, we::type::memory_buffer_info_t>
    memory_buffers;
  std::unordered_map<std::string, unsigned long> expected;
  expr::eval::context context;
  unsigned long total (0);

  for (std::string const& name : names)
  {
    if (memory_buffers.emplace
          ( std::piecewise_construct
          , std::forward_as_tuple (name)
          , std::forward_as_tuple ( "${" + name + "}"
                                  , fhg::util::testing::random_identifier()
                                  )
          ).second
       )
    {
      unsigned long const value
        {fhg::util::testing::random<unsigned long>()()};
      context.bind_and_discard_ref ({name}, value);
      expected.emplace (name, value);
      total += value;
    }
  }

  we::type::module_call_t const module_call
    ( fhg::util::testing::random_identifier()
    , fhg::util::testing::random_identifier()
    , std::move (memory_buffers)
    , {}
    , {}
    );

  BOOST_REQUIRE (equal (module_call.memory_buffer_sizes (context), expected));
  BOOST_REQUIRE_EQUAL (module_call.memory_buffer_size_total (context), total);
}

BOOST_AUTO_TEST_CASE (memory_buffer_alignments)
{
  std::list<std::string> names
    {fhg::util::testing::random<unsigned long>()() % 100};

  std::generate ( names.begin(), names.end()
                , []{ return fhg::util::testing::random_identifier(); }
                );

  std::unordered_map<std::string, we::type::memory_buffer_info_t>
    memory_buffers;
  std::unordered_map<std::string, unsigned long> expected;
  expr::eval::context context;

  for (std::string const& name : names)
  {
    if (memory_buffers.emplace
          ( std::piecewise_construct
          , std::forward_as_tuple (name)
          , std::forward_as_tuple ( fhg::util::testing::random_identifier()
                                  , "${" + name + "}"
                                  )
          ).second
       )
    {
      unsigned long const value 
        (1ul << fhg::util::testing::random<unsigned long>{}(10,0));
      context.bind_and_discard_ref ({name}, value);
      expected.emplace (name, value);
    }
  }

  we::type::module_call_t const module_call
    ( fhg::util::testing::random_identifier()
    , fhg::util::testing::random_identifier()
    , std::move (memory_buffers)
    , {}
    , {}
    );

  for (auto const& name_and_buffer_info : module_call.memory_buffers())
  {
    BOOST_REQUIRE_EQUAL
      ( name_and_buffer_info.second.alignment (context)
      , expected.at (name_and_buffer_info.first)
      );
  }
}
