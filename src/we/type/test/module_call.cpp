#include <we/type/module_call.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random_string.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (memory_buffer_sizes_no_buffers)
{
  we::type::module_call_t const module_call
    ( fhg::util::testing::random_identifier()
    , fhg::util::testing::random_identifier()
    , std::unordered_map<std::string, std::string> {}
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
    for (std::pair<Key, Value> const& l : lhs)
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

  std::unordered_map<std::string, std::string> memory_buffers;
  std::unordered_map<std::string, unsigned long> expected;
  expr::eval::context context;
  unsigned long total (0);

  for (std::string const& name : names)
  {
    if (memory_buffers.emplace (name, "${" + name + "}").second)
    {
      unsigned long const value {fhg::util::testing::random<unsigned long>()()};
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
