// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <util-generic/join.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>
#include <util-generic/ostream/modifier.hpp>

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
