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

#include <util-generic/testing/random.hpp>

#include <logging/message.hpp>

#include <iosfwd>

namespace fhg
{
  namespace logging
  {
    //! \note operator== and operator<< are not in an anonmyous
    //! namespace for lookup reasons: they are used from a boost
    //! namespace, so only an anonymous namespace in that boost
    //! namespace would be used.
    bool operator== (message const& lhs, message const& rhs);
    std::ostream& operator<< (std::ostream& os, message const& x);
  }

  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        template<>
          struct random_impl<logging::message, void>
        {
          logging::message operator()() const
          {
            random<std::string> random_string;
            return {random_string(), random_string()};
          }
        };
      }
    }
  }
}
