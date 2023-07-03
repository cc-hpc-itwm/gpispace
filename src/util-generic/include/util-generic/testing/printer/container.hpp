// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/print_container.hpp>
#include <util-generic/testing/printer/generic.hpp>

#include <functional>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace printer
      {
        template<typename T>
          constexpr std::ostream& print (std::ostream& os, T const& x)
        {
          return os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (x);
        }

        template<typename Container>
          join_reference<Container, std::string> container
            ( Container const& c
            , std::string const& prefix
            , std::string const& suffix
            )
        {
          return print_container
            (prefix, ", ", suffix, c, &print<Value<Container>>);
        }

#define FHG_UTIL_TESTING_PRINTER_CONTAINER_PRINTER(type_, prefix_, suffix_) \
        FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER                          \
          (<typename... Args>, type_<Args...>, os, c)                       \
        {                                                                   \
          os << fhg::util::testing::printer::container<type_<Args...>>      \
                  (c, prefix_, suffix_);                                    \
        }
      }
    }
  }
}
