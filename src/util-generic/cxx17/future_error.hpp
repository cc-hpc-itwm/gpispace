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

#include <future>

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      //! std::future_error in c++11 has a exposition only constructor
      //! from std::error_code. In c++17, this changed to
      //! std::future_errc. This means that in c++11 and c++14 there
      //! is no standard-compliant way to actually construct a
      //! future_error. Most implementations (at least libstdcxx)
      //! provide that exposition only constructor though, so that
      //! there is a way to actually construct it.
      std::future_error make_future_error (std::error_code const&);
      std::future_error make_future_error (std::future_errc const&);
    }
  }
}

#include <util-generic/cxx17/future_error.ipp>
