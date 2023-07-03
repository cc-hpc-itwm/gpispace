// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/printer/vector.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>

namespace fhg
{
  namespace util
  {
    BOOST_AUTO_TEST_CASE
      (apply_for_each_and_collect_exceptions_calls_for_each_element)
    {
      auto const values
        (testing::unique_randoms<std::vector<std::string>> (50));
      std::vector<std::size_t> seen_counter (values.size(), 0);
      apply_for_each_and_collect_exceptions
        ( values
        , [&] (std::string const& s)
          {
            ++*( seen_counter.begin()
               + (std::find (values.begin(), values.end(), s) - values.begin())
               );
          }
        );

      BOOST_REQUIRE_EQUAL
        (seen_counter, std::vector<std::size_t> (values.size(), 1));
    }

    BOOST_AUTO_TEST_CASE (throw_collected_exceptions_does_nothing_if_empty)
    {
      throw_collected_exceptions ({});
    }

    namespace
    {
      std::string random_cstr_comparable()
      {
        return testing::random<std::string>{}
          (testing::random<char>::any_without_zero());
      }
    }

    BOOST_AUTO_TEST_CASE
      (throw_collected_exceptions_concats_printed_and_rethrows_runtime_error)
    {
      auto const message (random_cstr_comparable());

      testing::require_exception
        ( [&]
          {
            throw_collected_exceptions
              ({std::make_exception_ptr (std::logic_error (message))});
          }
        , std::runtime_error (message)
        );

      testing::require_exception
        ( [&]
          {
            throw_collected_exceptions
              ({ std::make_exception_ptr (std::logic_error (message))
               , std::make_exception_ptr (std::runtime_error (message))
              });
          }
        , std::runtime_error (message + "\n" + message)
        );
    }

    BOOST_AUTO_TEST_CASE (throw_collected_exceptions_allows_building_exceptions)
    {
      auto const value (testing::random<int>{}());
      testing::require_exception
        ( [&]
          {
            throw_collected_exceptions
              ( std::vector<int> {value, value + 1}
              , [] (int i) { return std::to_string (i + 1); }
              );
          }
        , std::runtime_error
            (std::to_string (value + 1) + "\n" + std::to_string (value + 2))
        );
    }

    BOOST_AUTO_TEST_CASE
      (apply_for_each_and_collect_exceptions_finishs_calling_before_throwing)
    {
      std::vector<std::pair<bool, std::size_t>> please_throw
        {{true, 0}, {false, 0}, {true, 0}, {false, 0}};

      testing::require_exception
        ( [&]
          {
            apply_for_each_and_collect_exceptions
              ( please_throw
              , [&] (std::pair<bool, std::size_t>& entry)
                {
                  ++entry.second;
                  if (entry.first)
                  {
                    throw std::runtime_error
                      (std::to_string (&entry - please_throw.data()));
                  }
                }
              );
          }
        , std::runtime_error ("0\n2")
        );

      for (auto const& entry : please_throw)
      {
        BOOST_REQUIRE_EQUAL (entry.second, 1);
      }
    }

    BOOST_AUTO_TEST_CASE
      (wait_and_collect_exceptions_calls_get_and_collects)
    {
      std::vector<std::pair<bool, std::size_t>> please_throw
        {{true, 0}, {false, 0}, {true, 0}, {false, 0}};
      std::vector<std::future<void>> futures;

      for (auto& entry : please_throw)
      {
        auto* entry_ptr (&entry);

        futures.emplace_back
          ( std::async
              ( [&please_throw, entry_ptr]
                {
                  ++entry_ptr->second;
                  if (entry_ptr->first)
                  {
                    throw std::runtime_error
                      (std::to_string (entry_ptr - please_throw.data()));
                  }
                }
              )
          );
      }

      testing::require_exception
        ( [&]
          {
            wait_and_collect_exceptions (futures);
          }
        , std::runtime_error ("0\n2")
        );

      for (auto const& entry : please_throw)
      {
        BOOST_REQUIRE_EQUAL (entry.second, 1);
      }
    }
  }
}
