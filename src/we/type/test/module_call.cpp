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
    , std::unordered_map<std::string, we::type::memory_buffer_info>()
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

  std::unordered_map<std::string, we::type::memory_buffer_info> memory_buffers;
  std::unordered_map<std::string, unsigned long> expected;
  expr::eval::context context;
  unsigned long total (0);

  for (std::string const& name : names)
  {
    if ( memory_buffers.emplace
           ( std::piecewise_construct
           , std::make_tuple (name)
           , std::make_tuple
               ( "${" + name + "}"
               , fhg::util::testing::random_integral<unsigned long>()%2
               )
           ).second
       )
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

BOOST_AUTO_TEST_CASE (memory_buffer_info_is_stored)
{
  std::string const name (fhg::util::testing::random_string());
  std::string const size (fhg::util::testing::random_string());
  bool const read_only (fhg::util::testing::random_integral<unsigned long>()%2);

  std::unordered_map<std::string, we::type::memory_buffer_info> memory_buffers;

  memory_buffers.emplace
    ( std::piecewise_construct
    , std::make_tuple (name)
    , std::make_tuple (size, read_only)
    );

  we::type::module_call_t const module_call
    ( fhg::util::testing::random_identifier()
    , fhg::util::testing::random_identifier()
    , std::move (memory_buffers)
    , {}
    , {}
    );

  BOOST_REQUIRE_EQUAL (module_call.memory_buffers().size(), 1);
  BOOST_REQUIRE_EQUAL (module_call.memory_buffers().cbegin()->first, name);
  BOOST_REQUIRE_EQUAL (module_call.memory_buffers().cbegin()->second.size(), size);
  BOOST_REQUIRE_EQUAL (module_call.memory_buffers().cbegin()->second.read_only(), read_only);
}
