// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <string>

namespace fhg
{
  namespace util
  {
    //! Determine a human readable string representing the unit for
    //! the given \c std::chrono::duration::period. SI units are
    //! represented by their standard symbols. Otherwise, the period
    //! is represented as "[n]s" or "[n/d]s" if d != 1.
    template<typename Period>
      std::string chrono_unit_for_period();
  }
}

#include <util-generic/chrono_unit_for_period.ipp>
