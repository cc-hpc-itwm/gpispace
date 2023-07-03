// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <we/type/signature.hpp>
#include <we/type/signature/pnetc_compare.hpp>

#include <we/type/signature/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>

#include <functional>
#include <limits>
#include <vector>

namespace pnetc
{
  namespace type
  {
    namespace
    {
      template<typename T>
        std::vector<T> const special_values()
      {
        return { +std::numeric_limits<T>::min()
               , -std::numeric_limits<T>::min()
               , +std::numeric_limits<T>::lowest()
               , -std::numeric_limits<T>::lowest()
               , +std::numeric_limits<T>::max()
               , -std::numeric_limits<T>::max()
               , +std::numeric_limits<T>::epsilon()
               , -std::numeric_limits<T>::epsilon()
               , +std::numeric_limits<T>::round_error()
               , -std::numeric_limits<T>::round_error()
               , +std::numeric_limits<T>::infinity()
               , -std::numeric_limits<T>::infinity()
               , +std::numeric_limits<T>::quiet_NaN()
               , -std::numeric_limits<T>::quiet_NaN()
               , +std::numeric_limits<T>::denorm_min()
               , -std::numeric_limits<T>::denorm_min()
               , fhg::util::testing::random<T>{}()
               , fhg::util::testing::random<T>{}()
               , +T {0}
               , -T {0}
               };
      }
      std::vector<float> floats()
      {
        auto values (special_values<float>());
        values.emplace_back (10.0f);
        values.emplace_back (10.000001f);
        values.emplace_back (10.0000001f);
        return values;
      }
      std::vector<double> doubles()
      {
        return special_values<double>();
      }
    }

    BOOST_DATA_TEST_CASE
      ( pnetc_eq_for_float_equals_std_equal
      , floats() * floats()
      , x
      , y
      )
    {
      BOOST_CHECK_EQUAL (std::equal_to<float>{} (x, y), pnetc_eq{} (x, y));
    }
    BOOST_DATA_TEST_CASE
      ( pnetc_eq_for_double_equals_std_equal
      , doubles() * doubles()
      , x
      , y
      )
    {
      BOOST_CHECK_EQUAL (std::equal_to<double>{} (x, y), pnetc_eq{} (x, y));
    }
  }
}
