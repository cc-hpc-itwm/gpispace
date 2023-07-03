// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/print_exception.hpp>

#include <boost/test/unit_test_monitor.hpp>
#include <boost/test/unit_test_suite.hpp>

#include <sstream>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      //! \note HACK: relies on being called during exception handling
      //! due to needing the unchanged type for rethrowing. if called
      //! from a exception_ptr or alike, it will fail due to
      //! std::current_exception() being something else or null.
      struct flatten_nested_exceptions
      {
        flatten_nested_exceptions()
        {
          ::boost::unit_test::unit_test_monitor
            .register_exception_translator<std::exception>
              ( [] (std::exception const& ex)
                {
                  if (!dynamic_cast<std::nested_exception const*> (&ex))
                  {
                    throw;
                  }

                  struct flattened_exception : std::exception
                  {
                    std::string _what;
                    flattened_exception (std::string const& what)
                      : _what (what)
                    {}
                    const char* what() const noexcept override
                    {
                      return _what.c_str();
                    }
                  } flattened (fhg::util::current_exception_printer (": ").string());
                  throw flattened;
                }
              );
        }
      };
    }
  }
}

namespace
{
  using fhg::util::testing::flatten_nested_exceptions;
  BOOST_GLOBAL_FIXTURE (flatten_nested_exceptions);
}
