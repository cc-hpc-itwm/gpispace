// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/join.hpp>
#include <util-generic/ostream/modifier.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <functional>
#include <sstream>

namespace fhg
{
  namespace util
  {
    void throw_collected_exceptions
      (std::vector<std::exception_ptr> const& exceptions)
    {
      if (exceptions.empty())
      {
        return;
      }

      throw std::runtime_error
        ( join ( exceptions
               , '\n'
               , [] (std::ostream& os, std::exception_ptr const& ex)
                   -> std::ostream&
                 {
                   return os << exception_printer (ex, ": ");
                 }
               ).string()
        );
    }

    void wait_and_collect_exceptions (std::vector<std::future<void>>& futures)
    {
      apply_for_each_and_collect_exceptions
        (futures, std::bind (&std::future<void>::get, std::placeholders::_1));
    }
  }
}
