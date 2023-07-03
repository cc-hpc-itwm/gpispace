// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/cxx17/logical_operator_type_traits.hpp>
#include <util-generic/cxx17/void_t.hpp>
#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/require_compiletime.hpp>

#include <util-qt/qdatastream_enum.hpp>

#include <boost/test/unit_test.hpp>

QT_BEGIN_NAMESPACE

namespace
{
#define IS_VALID_EXPRESSION_TRAIT(name_, expected_ret_, expression_...) \
  template<typename T, typename = void>                                 \
    struct name_ : std::false_type {};                                  \
  template<typename T>                                                  \
    struct name_<T, fhg::util::cxx17::void_t<decltype (expression_)>>   \
      : std::is_same<expected_ret_, decltype (expression_)>             \
  {}

  IS_VALID_EXPRESSION_TRAIT
    ( has_qdatastream_in
    , QDataStream&
    , std::declval<QDataStream&>() << std::declval<T>()
    );
  IS_VALID_EXPRESSION_TRAIT
    ( has_qdatastream_out
    , QDataStream&
    , std::declval<QDataStream&>() >> std::declval<T&>()
    );
}

BOOST_AUTO_TEST_CASE (sfinaes_out_for_non_enums)
{
  struct S {};
  FHG_UTIL_TESTING_COMPILETIME_CHECK (!has_qdatastream_in<S>{});
  FHG_UTIL_TESTING_COMPILETIME_CHECK (!has_qdatastream_out<S>{});

  using V = void;
  FHG_UTIL_TESTING_COMPILETIME_CHECK (!has_qdatastream_in<V>{});
  FHG_UTIL_TESTING_COMPILETIME_CHECK (!has_qdatastream_out<V>{});
}

BOOST_AUTO_TEST_CASE (exists_for_enum)
{
  enum E {};
  FHG_UTIL_TESTING_COMPILETIME_CHECK (has_qdatastream_in<E>{});
  FHG_UTIL_TESTING_COMPILETIME_CHECK (has_qdatastream_out<E>{});

  enum class EC {};
  FHG_UTIL_TESTING_COMPILETIME_CHECK (has_qdatastream_in<EC>{});
  FHG_UTIL_TESTING_COMPILETIME_CHECK (has_qdatastream_out<EC>{});
}

namespace n1
{
  namespace n2
  {
    enum E {};

    namespace n3
    {
      BOOST_AUTO_TEST_CASE (works_in_sub_namespace)
      {
        FHG_UTIL_TESTING_COMPILETIME_CHECK (has_qdatastream_in<E>{});
        FHG_UTIL_TESTING_COMPILETIME_CHECK (has_qdatastream_out<E>{});
      }
    }
  }
  namespace n4
  {
    BOOST_AUTO_TEST_CASE (works_in_sibling_namespace)
    {
      FHG_UTIL_TESTING_COMPILETIME_CHECK (has_qdatastream_in<n2::E>{});
      FHG_UTIL_TESTING_COMPILETIME_CHECK (has_qdatastream_out<n2::E>{});
    }
  }
  BOOST_AUTO_TEST_CASE (works_in_parent_namespace)
  {
    FHG_UTIL_TESTING_COMPILETIME_CHECK (has_qdatastream_in<n2::E>{});
    FHG_UTIL_TESTING_COMPILETIME_CHECK (has_qdatastream_out<n2::E>{});
  }
}
BOOST_AUTO_TEST_CASE (works_in_top_level_namespace)
{
  FHG_UTIL_TESTING_COMPILETIME_CHECK (has_qdatastream_in<n1::n2::E>{});
  FHG_UTIL_TESTING_COMPILETIME_CHECK (has_qdatastream_out<n1::n2::E>{});
}

namespace
{
  enum class EC_64 : quint64
  {
    null = 0,
    one = 1,
    four_million_two_hundred_four_thousand_five_hundred_seventy_nine = 4204579,
  };

  enum class EC_16 : quint16
  {
    null = 0,
    one = 1,
    one_hundred_seventy_three = 173,
  };
}

FHG_BOOST_TEST_LOG_VALUE_PRINTER (EC_16, os, val)
{
  os << std::underlying_type<EC_16>::type (val);
}

BOOST_AUTO_TEST_CASE (writes_underlying_type)
{
  QByteArray ba;
  QDataStream os (&ba, QIODevice::WriteOnly);
  QDataStream is (&ba, QIODevice::ReadOnly);

  os << EC_64::null;
  os << EC_64::one;
  os << EC_64::four_million_two_hundred_four_thousand_five_hundred_seventy_nine;

  {
    quint64 raw_null;
    is >> raw_null;
    BOOST_CHECK_EQUAL (raw_null, 0);
  }
  {
    quint64 raw_one;
    is >> raw_one;
    BOOST_CHECK_EQUAL (raw_one, 1);
  }
  {
    quint64 raw_four_million_two_hundred_four_thousand_five_hundred_seventy_nine;
    is >> raw_four_million_two_hundred_four_thousand_five_hundred_seventy_nine;
    BOOST_CHECK_EQUAL (raw_four_million_two_hundred_four_thousand_five_hundred_seventy_nine, 4204579);
  }
}

BOOST_AUTO_TEST_CASE (reads_underlying_type)
{
  QByteArray ba;
  QDataStream os (&ba, QIODevice::WriteOnly);
  QDataStream is (&ba, QIODevice::ReadOnly);

  os << quint16 (0);
  os << quint16 (1);
  os << quint16 (173);

  {
    EC_16 null;
    is >> null;
    BOOST_CHECK_EQUAL (null, EC_16::null);
  }
  {
    EC_16 one;
    is >> one;
    BOOST_CHECK_EQUAL (one, EC_16::one);
  }
  {
    EC_16 one_hundred_seventy_three;
    is >> one_hundred_seventy_three;
    BOOST_CHECK_EQUAL (one_hundred_seventy_three, EC_16::one_hundred_seventy_three);
  }
}

QT_END_NAMESPACE
