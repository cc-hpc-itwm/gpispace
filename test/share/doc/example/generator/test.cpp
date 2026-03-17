// Copyright (C) 2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/source_directory.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/multimap.hpp>
#include <gspc/testing/printer/vector.hpp>
#include <gspc/testing/printer/we/type/value.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_container_is_permutation.hpp>

#include <gspc/we/type/value.hpp>
#include <gspc/testing/printer/we/type/value.hpp>

#include <boost/program_options.hpp>

#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <numeric>
#include <string>
#include <vector>

namespace
{
  template<typename T>
  void test_generator
    ( std::string const& network_name
    , unsigned long max_n
    , std::function<std::vector<T> (unsigned long)> expected
    )
  {
    ::boost::program_options::options_description options_description;
    options_description.add (gspc::testing::options::source_directory());
    options_description.add (gspc::testing::options::shared_directory());
    options_description.add (gspc::options::installation());
    options_description.add (gspc::options::drts());
    options_description.add (gspc::options::scoped_rifd());

    auto vm
      { gspc::testing::parse_command_line
          ( ::boost::unit_test::framework::master_test_suite().argc
          , ::boost::unit_test::framework::master_test_suite().argv
          , options_description
          )
      };

    gspc::util::temporary_path const shared_directory
      {gspc::testing::shared_directory (vm) / ("doc_example_generator_" + network_name)};
    gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
      {shared_directory, vm};
    gspc::installation const installation {vm};
    gspc::scoped_rifds const rifds
      { gspc::rifd::strategy {vm}
      , gspc::rifd::hostnames {vm}
      , gspc::rifd::port {vm}
      , installation
      };
    gspc::scoped_runtime_system const drts
      {vm, installation, "work:1", rifds.entry_points()};

    vm.notify();

    gspc::testing::make_net const make
      { installation
      , network_name
      , gspc::testing::source_directory (vm)
      };

    auto const n {gspc::testing::random<unsigned long>{} (max_n, 0UL)};

    auto const result
      { gspc::client (drts).put_and_run
          (gspc::workflow (make.pnet()), {{"n", n}})
      };

    auto const [begin, end] {result.equal_range ("id")};
    auto result_ids {std::vector<T>{}};
    std::transform
      ( begin, end
      , std::back_inserter (result_ids)
      , [] (auto const& result_id)
        {
          return ::boost::get<T> (result_id.second);
        }
      );

    GSPC_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION
      ( result_ids
      , expected (n)
      );
  }
}

BOOST_AUTO_TEST_CASE (generator_int)
{
  test_generator<int>
    ( "generator_int"
    , 1000UL
    , [] (unsigned long n)
      {
        auto expected {std::vector<int> (n)};
        std::iota (std::begin (expected), std::end (expected), 0);
        return expected;
      }
    );
}

BOOST_AUTO_TEST_CASE (generator_long)
{
  test_generator<long>
    ( "generator_long"
    , 1000UL
    , [] (unsigned long n)
      {
        auto expected {std::vector<long> (n)};
        std::iota (std::begin (expected), std::end (expected), 0L);
        return expected;
      }
    );
}

BOOST_AUTO_TEST_CASE (generator_uint)
{
  test_generator<unsigned int>
    ( "generator_uint"
    , 1000UL
    , [] (unsigned long n)
      {
        auto expected {std::vector<unsigned int> (n)};
        std::iota (std::begin (expected), std::end (expected), 0U);
        return expected;
      }
    );
}

BOOST_AUTO_TEST_CASE (generator_ulong)
{
  test_generator<unsigned long>
    ( "generator_ulong"
    , 1000UL
    , [] (unsigned long n)
      {
        auto expected {std::vector<unsigned long> (n)};
        std::iota (std::begin (expected), std::end (expected), 0UL);
        return expected;
      }
    );
}

BOOST_AUTO_TEST_CASE (generator_char)
{
  // Char generator starts at 'a' (97) and overflows at char max (127 for signed char)
  // so we can only get about 31 unique values before overflow
  test_generator<char>
    ( "generator_char"
    , 30UL
    , [] (unsigned long n)
      {
        auto expected {std::vector<char> (n)};
        std::iota (std::begin (expected), std::end (expected), 'a');
        return expected;
      }
    );
}

BOOST_AUTO_TEST_CASE (generator_string)
{
  test_generator<std::string>
    ( "generator_string"
    , 1000UL
    , [] (unsigned long n)
      {
        auto const increment
          { [] (std::string& value)
            {
              for ( auto symbol {std::rbegin (value)}
                  ; symbol != std::rend (value)
                  ; ++symbol
                  )
              {
                if (*symbol < 'z')
                {
                  ++(*symbol);

                  return;
                }

                *symbol = 'a';
              }

              value = "a" + value;
            }
          };

        auto expected {std::vector<std::string>{}};
        std::generate_n
          ( std::back_inserter (expected)
          , n
          , [&, x = std::string {"a"}]() mutable
            {
              auto current {x};
              increment (x);
              return current;
            }
          );
        return expected;
      }
    );
}

BOOST_AUTO_TEST_CASE (generator_bigint)
{
  test_generator<gspc::pnet::type::value::bigint_type>
    ( "generator_bigint"
    , 1000UL
    , [] (unsigned long n)
      {
        auto expected {std::vector<gspc::pnet::type::value::bigint_type> (n)};
        std::iota ( std::begin (expected), std::end (expected)
                  , gspc::pnet::type::value::bigint_type {0}
                  );
        return expected;
      }
    );
}
