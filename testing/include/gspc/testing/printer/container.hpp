// Copyright (C) 2015-2016,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/print_container.hpp>
#include <gspc/testing/printer/generic.hpp>

#include <functional>
#include <string>




      namespace gspc::testing::printer
      {
        template<typename T>
          constexpr std::ostream& print (std::ostream& os, T const& x)
        {
          return os << GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (x);
        }

        template<typename Container>
          gspc::util::join_reference<Container, std::string> container
            ( Container const& c
            , std::string const& prefix
            , std::string const& suffix
            )
        {
          return gspc::util::print_container
            (prefix, ", ", suffix, c, &print<gspc::util::Value<Container>>);
        }

#define GSPC_TESTING_PRINTER_CONTAINER_PRINTER(type_, prefix_, suffix_)     \
        GSPC_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER                         \
          (<typename... Args>, type_<Args...>, os, c)                       \
        {                                                                   \
          os << gspc::testing::printer::container<type_<Args...>>           \
                  (c, prefix_, suffix_);                                    \
        }
      }
