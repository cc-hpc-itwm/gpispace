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

#pragma once

#include <chrono>
#include <iostream>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace timer
    {
      //! Print START and DONE of a scope including the description and
      //! the execution time with given resolution.

      template<typename Duration, typename Clock = std::chrono::steady_clock>
        struct scoped
      {
        scoped (std::string, std::ostream& = std::cout);
        ~scoped();

      private:
        std::ostream& _os;
        std::string const _description;
        typename Clock::time_point const _start;
      };
    }
  }
}

#include <util-generic/timer/scoped.ipp>
