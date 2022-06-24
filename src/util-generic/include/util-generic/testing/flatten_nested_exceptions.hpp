// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
